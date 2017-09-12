#ifdef _WIN32

#include "../file_operations.h"
#include "win_extend_path_limit.h"
#include <system_error>         // std::system_error
#include <ctime>                // std::time_t
#include <Windows.h>            // WinAPI

namespace filesystem
{
    bool isExists(const Path &path)
    {
        const auto result = GetFileAttributesW(MAKE_EXTENDED_PATH(path).c_str());

        if (result == INVALID_FILE_ATTRIBUTES)
        {
            const auto err = GetLastError();
            switch (err)
            {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    return false;
                default:
                    throw std::system_error(GetLastError(), std::system_category());
            }
        }

        return true;
    }


    FileType getFileType(const Path &path)
    {
        const auto result = GetFileAttributesW(MAKE_EXTENDED_PATH(path).c_str());

        if (result == INVALID_FILE_ATTRIBUTES)
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        return (result & FILE_ATTRIBUTE_DIRECTORY) ? FileType::directory : FileType::file;
    }


    std::uint64_t getFileSize(const Path &path)
    {
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;

        if (!GetFileAttributesExW(MAKE_EXTENDED_PATH(path).c_str(),
                                  GetFileExInfoStandard,
                                  &fileInfo))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        ULARGE_INTEGER result;
        result.LowPart = fileInfo.nFileSizeLow;
        result.HighPart = fileInfo.nFileSizeHigh;

        return result.QuadPart;
    }


    std::chrono::system_clock::time_point getModifyDate(const Path &path)
    {
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;

        if (!GetFileAttributesExW(MAKE_EXTENDED_PATH(path).c_str(),
                                  GetFileExInfoStandard,
                                  &fileInfo))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        ULARGE_INTEGER result;
        result.LowPart = fileInfo.ftLastWriteTime.dwLowDateTime;
        result.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;

        return std::chrono::system_clock::from_time_t(result.QuadPart / 10000000ULL - 11644473600ULL);
    }


    void rename(const Path &oldPath, const Path &newPath)
    {
        if (!MoveFileW(MAKE_EXTENDED_PATH(oldPath).c_str(), MAKE_EXTENDED_PATH(newPath).c_str()))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }
    }
}

#else   // #ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  // #ifdef _WIN32
