#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef NEBULA_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#endif

namespace nebula {

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
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
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
