#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <filesystem>

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
