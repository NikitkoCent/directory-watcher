#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "path.h"
#include <cstdint>  // std::uint64_t
#include <chrono>   // std::chrono::time_point

namespace filesystem
{
    enum class FileType{ file, directory, other };

    /*
     * Returns true if file with specified path is exist; false otherwise
     * Parameters
     *  path    -   full path to file
     */
    bool isExists(const Path &path);

    /*
     * Returns type of file with specified path
     * Parameters
     *  path    -   full path to file
     *
     * Throws:
     *  std::system_error   -   any system error occured (for example, the file does not exists)
     */
    FileType getFileType(const Path &path);

    /*
     * For files : returns size in bytes
     * Otherwise (directories, etc.) : returns unspecified number
     *
     * Parameters
     *  path    -   full path to file
     *
     * Throws:
     *  std::system_error   -   any system error occured (for example, the file does not exists)
     */
    std::uint64_t getFileSize(const Path &path);

    /*
     * Returns last file modify date
     *
     * Parameters
     *  path    -   full path to file
     *
     * Throws:
     *  std::system_error   -   any system error occured (for example, the file does not exists)
     */
    std::chrono::system_clock::time_point getModifyDate(const Path &path);

    void rename(const Path &oldPath, const Path &newPath);
}

#endif // FILE_OPERATIONS_H
