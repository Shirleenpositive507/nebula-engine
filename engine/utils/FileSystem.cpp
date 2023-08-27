#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdio>

#ifdef NEBULA_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#include <fileapi.h>
#endif

namespace nebula {

    namespace fs = std::filesystem;

    MappedFile::MappedFile()
#ifdef NEBULA_PLATFORM_WINDOWS
        : m_handle(nullptr)
        , m_mapping(nullptr)
#endif
        , m_data(nullptr)
        , m_size(0) {}

    MappedFile::~MappedFile() {
        close();
    }

    bool MappedFile::open(const std::string& path) {
        close();
#ifdef NEBULA_PLATFORM_WINDOWS
        m_handle = CreateFileA(
            path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );
        if (m_handle == INVALID_HANDLE_VALUE) {
            m_handle = nullptr;
            return false;
        }

        LARGE_INTEGER fileSize;
        GetFileSizeEx(m_handle, &fileSize);
        m_size = static_cast<size_t>(fileSize.QuadPart);

        m_mapping = CreateFileMapping(m_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!m_mapping) {
            CloseHandle(m_handle);
            m_handle = nullptr;
            return false;
        }

        m_data = static_cast<uint8_t*>(MapViewOfFile(m_mapping, FILE_MAP_READ, 0, 0, 0));
        if (!m_data) {
            CloseHandle(m_mapping);
            CloseHandle(m_handle);
            m_mapping = nullptr;
            m_handle = nullptr;
            return false;
        }
        return true;
#else
        return false;
#endif
    }

    void MappedFile::close() {
#ifdef NEBULA_PLATFORM_WINDOWS
        if (m_data) { UnmapViewOfFile(m_data); m_data = nullptr; }
        if (m_mapping) { CloseHandle(m_mapping); m_mapping = nullptr; }
        if (m_handle) { CloseHandle(m_handle); m_handle = nullptr; }
#endif
        m_size = 0;
    }

    bool MappedFile::isOpen() const {
        return m_data != nullptr;
    }

    const uint8_t* MappedFile::data() const {
        return m_data;
    }

    uint8_t* MappedFile::data() {
        return m_data;
    }

    size_t MappedFile::size() const {
        return m_size;
    }

    FileLock::FileLock()
#ifdef NEBULA_PLATFORM_WINDOWS
        : m_handle(nullptr)
#endif
        , m_locked(false) {}

    FileLock::~FileLock() {
        unlock();
    }

    bool FileLock::lock(const std::string& path, bool shared) {
        unlock();
#ifdef NEBULA_PLATFORM_WINDOWS
        DWORD access = shared ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
        DWORD shareMode = shared ? FILE_SHARE_READ : 0;
        m_handle = CreateFileA(
            path.c_str(), access, shareMode, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );
        if (m_handle == INVALID_HANDLE_VALUE) {
            m_handle = nullptr;
            return false;
        }
        OVERLAPPED overlapped = {0};
        if (!LockFileEx(m_handle, shared ? LOCKFILE_FAIL_IMMEDIATELY : 0, 0, MAXDWORD, MAXDWORD, &overlapped)) {
            CloseHandle(m_handle);
            m_handle = nullptr;
            return false;
        }
        m_path = path;
        m_locked = true;
        return true;
#else
        return false;
#endif
    }

    void FileLock::unlock() {
#ifdef NEBULA_PLATFORM_WINDOWS
        if (m_handle) {
            OVERLAPPED overlapped = {0};
            UnlockFileEx(m_handle, 0, MAXDWORD, MAXDWORD, &overlapped);
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
#endif
        m_locked = false;
        m_path.clear();
    }

    bool FileLock::isLocked() const {
        return m_locked;
    }

    DirectoryWatcher::DirectoryWatcher() {}
    DirectoryWatcher::~DirectoryWatcher() {}

    void DirectoryWatcher::watchDirectory(const std::string& dirPath, bool recursive) {
        if (!FileSystem::directoryExists(dirPath)) return;

        WatchEntry entry;
        entry.path = dirPath;
        entry.recursive = recursive;

        try {
            for (const auto& file : fs::recursive_directory_iterator(dirPath)) {
                if (file.is_regular_file()) {
                    entry.fileTimes[file.path().string()] = fs::last_write_time(file.path());
                }
            }
        } catch (const std::exception&) {
        }

        m_entries.push_back(std::move(entry));
    }

    void DirectoryWatcher::unwatchDirectory(const std::string& dirPath) {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                [&](const WatchEntry& e) { return e.path == dirPath; }),
            m_entries.end()
        );
    }

    void DirectoryWatcher::unwatchAll() {
        m_entries.clear();
    }

    void DirectoryWatcher::poll() {
        for (auto& entry : m_entries) {
            std::unordered_map<std::string, std::filesystem::file_time_type> current;

            try {
                auto begin = fs::recursive_directory_iterator(entry.path);
                auto end = fs::recursive_directory_iterator();
                for (auto it = begin; it != end; ++it) {
                    if (it->is_regular_file()) {
                        std::string path = it->path().string();
                        auto time = fs::last_write_time(it->path());
                        current[path] = time;

                        auto prev = entry.fileTimes.find(path);
                        if (prev == entry.fileTimes.end()) {
                            if (m_onCreated) m_onCreated(path, false);
                        } else if (prev->second != time) {
                            if (m_onModified) m_onModified(path, false);
                        }
                    }
                }
            } catch (...) {}

            for (auto& [path, _] : entry.fileTimes) {
                if (current.find(path) == current.end()) {
                    if (m_onDeleted) m_onDeleted(path, false);
                }
            }

            entry.fileTimes = std::move(current);
        }
    }

    void DirectoryWatcher::setFileCreatedCallback(Callback callback) {
        m_onCreated = std::move(callback);
    }

    void DirectoryWatcher::setFileModifiedCallback(Callback callback) {
        m_onModified = std::move(callback);
    }

    void DirectoryWatcher::setFileDeletedCallback(Callback callback) {
        m_onDeleted = std::move(callback);
    }

    namespace fs = std::filesystem;

    std::string FileSystem::readFile(const std::string& path) {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) return std::string();
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    bool FileSystem::writeFile(const std::string& path, const std::string& data) {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) return false;
        file.write(data.data(), data.size());
        return file.good();
    }

    std::vector<uint8_t> FileSystem::readBinary(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return {};
        file.seekg(0, std::ios::end);
        std::streampos sizePos = file.tellg();
        if (sizePos <= 0) return {};
        size_t size = static_cast<size_t>(sizePos);
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
        if (!file) {
            data.resize(static_cast<size_t>(file.gcount()));
        }
        return data;
    }

    bool FileSystem::writeBinary(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return false;
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }

    bool FileSystem::fileExists(const std::string& path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }

    bool FileSystem::directoryExists(const std::string& path) {
        return fs::exists(path) && fs::is_directory(path);
    }

    bool FileSystem::createDirectory(const std::string& path) {
        return fs::create_directory(path);
    }

    bool FileSystem::createDirectories(const std::string& path) {
        return fs::create_directories(path);
    }

    bool FileSystem::deleteFile(const std::string& path) {
        return fs::remove(path);
    }

    bool FileSystem::deleteDirectory(const std::string& path) {
        return fs::remove_all(path) > 0;
    }

    bool FileSystem::copyFile(const std::string& from, const std::string& to) {
        return fs::copy_file(from, to, fs::copy_options::overwrite_existing);
    }

    bool FileSystem::copyDirectory(const std::string& from, const std::string& to) {
        fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        return true;
    }

    bool FileSystem::moveFile(const std::string& from, const std::string& to) {
        fs::rename(from, to);
        return !fs::exists(from) && fs::exists(to);
    }

    bool FileSystem::moveDirectory(const std::string& from, const std::string& to) {
        fs::rename(from, to);
        return !fs::exists(from) && fs::exists(to);
    }

    std::string FileSystem::getFileName(const std::string& path) {
        return fs::path(path).filename().string();
    }

    std::string FileSystem::getFileExtension(const std::string& path) {
        return fs::path(path).extension().string();
    }

    std::string FileSystem::getFileNameWithoutExtension(const std::string& path) {
        return fs::path(path).stem().string();
    }

    std::string FileSystem::getDirectoryName(const std::string& path) {
        return fs::path(path).parent_path().string();
    }

    std::string FileSystem::getAbsolutePath(const std::string& path) {
        return fs::absolute(path).string();
    }

    uint64_t FileSystem::getFileSize(const std::string& path) {
        return fs::file_size(path);
    }

    std::filesystem::file_time_type FileSystem::getLastWriteTime(const std::string& path) {
        return fs::last_write_time(path);
    }

    std::vector<std::string> FileSystem::listFiles(const std::string& directory, const std::string& pattern) {
        std::vector<std::string> result;
        if (!directoryExists(directory)) return result;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string name = entry.path().filename().string();
                if (pattern == "*") {
                    result.push_back(name);
                } else {
                    std::string ext = entry.path().extension().string();
                    if (ext == pattern) {
                        result.push_back(name);
                    }
                }
            }
        }
        return result;
    }

    std::vector<std::string> FileSystem::listDirectories(const std::string& directory) {
        std::vector<std::string> result;
        if (!directoryExists(directory)) return result;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_directory()) {
                result.push_back(entry.path().filename().string());
            }
        }
        return result;
    }

    std::string FileSystem::getCurrentDirectory() {
        return fs::current_path().string();
    }

    bool FileSystem::setCurrentDirectory(const std::string& path) {
        fs::current_path(path);
        return true;
    }

    std::string FileSystem::getSpecialFolder(SpecialFolder folder) {
#ifdef NEBULA_PLATFORM_WINDOWS
        int csidl = 0;
        switch (folder) {
            case SpecialFolder::Desktop:   csidl = CSIDL_DESKTOP; break;
            case SpecialFolder::Documents: csidl = CSIDL_PERSONAL; break;
            case SpecialFolder::AppData:   csidl = CSIDL_APPDATA; break;
            case SpecialFolder::Temp:      return fs::temp_directory_path().string();
        }
        char path[MAX_PATH];
        if (SHGetFolderPathA(nullptr, csidl, nullptr, 0, path) == S_OK) {
            return std::string(path);
        }
        return std::string();
#else
        switch (folder) {
            case SpecialFolder::Desktop:   return getenv("HOME") + std::string("/Desktop");
            case SpecialFolder::Documents: return getenv("HOME") + std::string("/Documents");
            case SpecialFolder::AppData:   return getenv("HOME") + std::string("/.local/share");
            case SpecialFolder::Temp:      return "/tmp";
        }
        return std::string();
#endif
    }

    std::string FileSystem::sanitizePath(const std::string& path) {
        std::string result = path;
        std::replace(result.begin(), result.end(), '\\', '/');

        while (result.find("//") != std::string::npos) {
            result.replace(result.find("//"), 2, "/");
        }

        size_t pos;
        while ((pos = result.find("/./")) != std::string::npos) {
            result.erase(pos, 2);
        }

        return result;
    }

    std::string FileSystem::createTempFile(const std::string& prefix, const std::string& suffix) {
        std::string tmpDir = fs::temp_directory_path().string();
        std::string templatePath = tmpDir + "/" + prefix + "XXXXXX" + suffix;

#ifdef NEBULA_PLATFORM_WINDOWS
        char tempPath[MAX_PATH];
        GetTempFileNameA(tmpDir.c_str(), prefix.c_str(), 0, tempPath);
        return std::string(tempPath);
#else
        std::vector<char> buf(templatePath.begin(), templatePath.end());
        buf.push_back('\0');
        int fd = mkstemp(buf.data());
        if (fd != -1) {
            close(fd);
            return std::string(buf.data());
        }
        return std::string();
#endif
    }

    std::string FileSystem::createTempDirectory(const std::string& prefix) {
        std::string tmpDir = fs::temp_directory_path().string();
        std::string dirPath = tmpDir + "/" + prefix + "XXXXXX";

#ifdef NEBULA_PLATFORM_WINDOWS
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        char dirName[MAX_PATH];
        GetTempFileNameA(tempPath, prefix.c_str(), 0, dirName);
        fs::remove(dirName);
        fs::create_directory(dirName);
        return std::string(dirName);
#else
        std::vector<char> buf(dirPath.begin(), dirPath.end());
        buf.push_back('\0');
        char* result = mkdtemp(buf.data());
        if (result) {
            return std::string(result);
        }
        return std::string();
#endif
    }

    void FileWatcher::watch(const std::string& path) {
        if (!FileSystem::fileExists(path)) return;
        FileWatcherEntry entry;
        entry.path = path;
        entry.lastWriteTime = FileSystem::getLastWriteTime(path);
        m_entries.push_back(entry);
    }

    void FileWatcher::unwatch(const std::string& path) {
        m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
            [&](const FileWatcherEntry& e) { return e.path == path; }), m_entries.end());
    }

    void FileWatcher::unwatchAll() {
        m_entries.clear();
    }

    void FileWatcher::poll() {
        for (auto& entry : m_entries) {
            auto currentTime = FileSystem::getLastWriteTime(entry.path);
            if (currentTime != entry.lastWriteTime) {
                entry.lastWriteTime = currentTime;
                if (m_callback) {
                    m_callback(entry.path);
                }
            }
        }
    }

    void FileWatcher::setCallback(Callback callback) {
        m_callback = std::move(callback);
    }

}
