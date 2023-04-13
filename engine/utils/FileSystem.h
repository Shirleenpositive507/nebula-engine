#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <cstdint>

namespace nebula {

    enum class SpecialFolder {
        Desktop,
        Documents,
        AppData,
        Temp
    };

    struct FileWatcherEntry {
        std::string path;
        std::filesystem::file_time_type lastWriteTime;
    };

    class MappedFile {
    public:
        MappedFile();
        ~MappedFile();

        bool open(const std::string& path);
        void close();
        bool isOpen() const;

        const uint8_t* data() const;
        uint8_t* data();
        size_t size() const;

    private:
#ifdef NEBULA_PLATFORM_WINDOWS
        void* m_handle;
        void* m_mapping;
#endif
        uint8_t* m_data;
        size_t m_size;
    };

    class FileLock {
    public:
        FileLock();
        ~FileLock();

        bool lock(const std::string& path, bool shared = false);
        void unlock();
        bool isLocked() const;

    private:
#ifdef NEBULA_PLATFORM_WINDOWS
        void* m_handle;
#endif
        std::string m_path;
        bool m_locked;
    };

    class DirectoryWatcher {
    public:
        using Callback = std::function<void(const std::string& path, bool isDir)>;

        DirectoryWatcher();
        ~DirectoryWatcher();

        void watchDirectory(const std::string& dirPath, bool recursive = false);
        void unwatchDirectory(const std::string& dirPath);
        void unwatchAll();
        void poll();

        void setFileCreatedCallback(Callback callback);
        void setFileModifiedCallback(Callback callback);
        void setFileDeletedCallback(Callback callback);

    private:
        struct WatchEntry {
            std::string path;
            bool recursive;
            std::unordered_map<std::string, std::filesystem::file_time_type> fileTimes;
        };

        std::vector<WatchEntry> m_entries;
        Callback m_onCreated;
        Callback m_onModified;
        Callback m_onDeleted;
    };

    class FileSystem {
    public:
        static std::string readFile(const std::string& path);
        static bool writeFile(const std::string& path, const std::string& data);

        static std::vector<uint8_t> readBinary(const std::string& path);
        static bool writeBinary(const std::string& path, const std::vector<uint8_t>& data);

        static bool fileExists(const std::string& path);
        static bool directoryExists(const std::string& path);

        static bool createDirectory(const std::string& path);
        static bool createDirectories(const std::string& path);

        static bool deleteFile(const std::string& path);
        static bool deleteDirectory(const std::string& path);

        static bool copyFile(const std::string& from, const std::string& to);
        static bool copyDirectory(const std::string& from, const std::string& to);

        static bool moveFile(const std::string& from, const std::string& to);
        static bool moveDirectory(const std::string& from, const std::string& to);

        static std::string getFileName(const std::string& path);
        static std::string getFileExtension(const std::string& path);
        static std::string getFileNameWithoutExtension(const std::string& path);
        static std::string getDirectoryName(const std::string& path);

        static std::string getAbsolutePath(const std::string& path);

        static uint64_t getFileSize(const std::string& path);

        static std::filesystem::file_time_type getLastWriteTime(const std::string& path);

        static std::vector<std::string> listFiles(const std::string& directory, const std::string& pattern = "*");
        static std::vector<std::string> listDirectories(const std::string& directory);

        static std::string getCurrentDirectory();
        static bool setCurrentDirectory(const std::string& path);

        static std::string getSpecialFolder(SpecialFolder folder);

        static std::string sanitizePath(const std::string& path);
        static std::string createTempFile(const std::string& prefix = "", const std::string& suffix = ".tmp");
        static std::string createTempDirectory(const std::string& prefix = "");
    };

    class FileWatcher {
    public:
        using Callback = std::function<void(const std::string& path)>;

        void watch(const std::string& path);
        void unwatch(const std::string& path);
        void unwatchAll();
        void poll();
        void setCallback(Callback callback);

    private:
        std::vector<FileWatcherEntry> m_entries;
        Callback m_callback;
    };

}
