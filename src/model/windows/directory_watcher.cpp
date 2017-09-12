#ifdef _WIN32

#include "../directory_watcher.h"
#include "../ordered_set.h"
#include "win_extend_path_limit.h"
#include <utility>                  // std::move, etc.
#include <memory>                   // std::unique_ptr, etc.
#include <stdexcept>                // std::runtime_error, std::out_of_range
#include <system_error>             // std::system_error
#include <atomic>                   // std::atomic_bool
#include <type_traits>              // std::aligned_storage
#include <stack>                    // std::stack
#include <iterator>                 // std::distance
#include <cstring>                  // std::memset
#include <cstddef>                  // std::size_t
#include <cstdint>                  // std::int8_t
#include <cwchar>                   // std::wcsncmp
#include <Windows.h>                // WinAPI


/*
 * Work scheme:
 * 1. Full directory scanning (see fullFilesReupdate method)
 * 2. Notify
 * 3. ReadDirectoryChanges (see updateChangesList method)
 * 4. if p.3 interrupted (see Impl::stopWatch method) -> loop break
 * 5. if system buffer from p.3 overflows (see refillFilesList method)
 *      -> full directory rescanning (see fullFilesReupdate method)
 * 6. back to p.2
 */


class DirectoryWatcher::Impl
{
public:
    Impl(DirectoryWatcher &parent, const filesystem::Path &path)
        : parent(parent), path(path), searchPath(createSearchPath()),
          dirHandle(createDirHandle()),
          ioEvent(createEvent()), breakEvent(createEvent()),
          winAPIChanges(std::make_unique<WinAPIChangesBuffer>()),
          needBreak(false)
    {
    }

    Impl(DirectoryWatcher &parent, filesystem::Path &&path)
        : parent(parent), path(path), searchPath(createSearchPath()),
          dirHandle(createDirHandle()),
          ioEvent(createEvent()), breakEvent(createEvent()),
          winAPIChanges(std::make_unique<WinAPIChangesBuffer>()),
          needBreak(false)
    {
    }


    void startWatch()
    {
        fullFilesReupdate();
        notify();

        while (!needBreak)
        {
            updateChangesList();
            updateFilesList();

            if (needBreak)
            {                
                break;
            }

            notify();
        }

        parent.handler = nullptr;
        needBreak.store(false);
    }

    void stopWatch()
    {
        needBreak.store(true);
        if (!SetEvent(breakEvent.getHandle()))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }
    }


    const filesystem::Path& getPath() const
    {
        return path;
    }


private:
    class RAIIHandle
    {
    public:
        RAIIHandle(HANDLE handle) : handle(handle) {}
        ~RAIIHandle() { CloseHandle(handle); handle = INVALID_HANDLE_VALUE; }

        HANDLE getHandle() const { return handle; }

    private:
        HANDLE handle;
    };

    static constexpr std::size_t winAPIChangesBufferSize = 64 * 1024;   //64 KB is network limitation
    using WinAPIChangesBuffer = typename std::aligned_storage<winAPIChangesBufferSize, alignof(DWORD)>::type;


    DirectoryWatcher &parent;
    const filesystem::Path path;
    const filesystem::Path searchPath;
    const RAIIHandle dirHandle;
    const RAIIHandle ioEvent, breakEvent;
    const std::unique_ptr<WinAPIChangesBuffer> winAPIChanges;
    DirectoryWatcher::ChangeContainer changes;
    OrderedSet<filesystem::Path> files;
    std::atomic_bool needBreak;


    static inline filesystem::Path getPathFromRaw(const WCHAR *path, DWORD sizeInBytes)
    {
        return {path, path + sizeInBytes / sizeof(WCHAR)};
    }


    HANDLE createDirHandle()
    {
        const HANDLE result = CreateFileW(MAKE_EXTENDED_PATH(path).c_str(),
                                          FILE_LIST_DIRECTORY,
                                          FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          NULL,
                                          OPEN_EXISTING,
                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                          NULL
                                         );

        if (result == INVALID_HANDLE_VALUE)
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        return result;
    }

    HANDLE createEvent()
    {
        const HANDLE result = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (result == NULL)
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        return result;
    }


    filesystem::Path createSearchPath()
    {
        filesystem::Path result = path / L"*";
        return std::move(MAKE_EXTENDED_PATH(result));
    }


    void fullFilesReupdate()
    {
        changes.clear();
        generateAllFilesRemoved();
        refillFilesList();
        generateAllFilesAdded();
    }

    void generateAllFilesRemoved()
    {
        ChangeEntry::IndexType i = 0;
        for (const auto &file : files)
        {
            changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::remove, i++,
                                             file, filesystem::Path(), false));
        }
    }

    void refillFilesList()
    {
        files.clear();
        WIN32_FIND_DATA findFileData;

        auto hFind = FindFirstFileW(searchPath.getPathString().c_str(), &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            const auto err = GetLastError();
            if (err != ERROR_FILE_NOT_FOUND)
            {
                throw std::system_error(err, std::system_category());
            }

            return;
        }

        try
        {
            do
            {
                if (std::wcsncmp(findFileData.cFileName, L".", 2) == 0)
                {
                    continue;
                }
                if (std::wcsncmp(findFileData.cFileName, L"..", 3) == 0)
                {
                    continue;
                }

                files.emplace_back(findFileData.cFileName);
            }
            while (FindNextFile(hFind, &findFileData) != 0);

            const auto err = GetLastError();
            if ((err != ERROR_SUCCESS) && (err != ERROR_NO_MORE_FILES))     // system buffer overflows
            {
                throw std::system_error(err, std::system_category());
            }

            FindClose(hFind);
        }
        catch (...)
        {
            FindClose(hFind);
            throw;
        }
    }

    void generateAllFilesAdded()
    {
        ChangeEntry::IndexType i = 0;
        for (const auto &file : files)
        {
            changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::add, i++,
                                             filesystem::Path(), file, false));
        }
    }


    void updateChangesList()
    {
        changes.clear();

        OVERLAPPED overlapInfo;

        std::memset(&overlapInfo, 0, sizeof(overlapInfo));
        overlapInfo.hEvent = ioEvent.getHandle();

        if (!ReadDirectoryChangesW(dirHandle.getHandle(),
                                   winAPIChanges.get(),
                                   winAPIChangesBufferSize,
                                   FALSE,
                                   FILE_NOTIFY_CHANGE_FILE_NAME
                                   | FILE_NOTIFY_CHANGE_DIR_NAME
                                   | FILE_NOTIFY_CHANGE_SIZE
                                   | FILE_NOTIFY_CHANGE_LAST_WRITE,
                                   NULL,
                                   &overlapInfo,
                                   NULL
                                   ))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        HANDLE handles[2]{ breakEvent.getHandle(), ioEvent.getHandle() };
        static_assert(2 <= MAXIMUM_WAIT_OBJECTS, "count of wait objects > MAXIMUM_WAIT_OBJECTS");
        switch (WaitForMultipleObjects(2, handles, FALSE, INFINITE))
        {
            case WAIT_FAILED:
                throw std::system_error(GetLastError(), std::system_category());
            case WAIT_OBJECT_0:
                if (!CancelIo(dirHandle.getHandle()))
                {
                    throw std::system_error(GetLastError(), std::system_category());
                }
                break;
            case WAIT_OBJECT_0 + 1:
                handleReadChangesResults(overlapInfo);
                break;
            default:
                throw std::runtime_error("Unexpected result of a system call");
        }
    }

    void handleReadChangesResults(OVERLAPPED &overlapInfo)
    {
        DWORD bytesTransferred;

        if (!GetOverlappedResult(dirHandle.getHandle(), &overlapInfo, &bytesTransferred, TRUE))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }

        if ((bytesTransferred == 0) || (GetLastError() == ERROR_NOTIFY_ENUM_DIR))
        {
            fullFilesReupdate();
        }
        else
        {
            const std::int8_t *buffer = reinterpret_cast<std::int8_t *>(winAPIChanges.get());
            const FILE_NOTIFY_INFORMATION *notifies = nullptr;
            std::stack<filesystem::Path> renameOldNames;
            std::stack<filesystem::Path> renameNewNames;

            do
            {
                notifies = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(buffer);

                switch (notifies->Action)
                {
                    case FILE_ACTION_ADDED:
                        changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::add, 0,
                                                         filesystem::Path(),
                                                         getPathFromRaw(notifies->FileName, notifies->FileNameLength),
                                                         false));
                        break;
                    case FILE_ACTION_REMOVED:
                        changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::remove, 0,
                                                         getPathFromRaw(notifies->FileName, notifies->FileNameLength),
                                                         filesystem::Path(),
                                                         false));
                        break;
                    case FILE_ACTION_MODIFIED:
                        changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::modify, 0,
                                                         filesystem::Path(),
                                                         getPathFromRaw(notifies->FileName, notifies->FileNameLength),
                                                         false));
                        break;
                    case FILE_ACTION_RENAMED_OLD_NAME:
                        if (renameNewNames.empty())
                        {
                            renameOldNames.emplace(std::move(getPathFromRaw(notifies->FileName, notifies->FileNameLength)));
                        }
                        else
                        {                            
                            changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::rename, 0,
                                                             getPathFromRaw(notifies->FileName, notifies->FileNameLength),
                                                             std::move(renameNewNames.top()),
                                                             false));
                            renameNewNames.pop();
                        }

                        break;
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        if (renameOldNames.empty())
                        {
                            renameNewNames.emplace(std::move(getPathFromRaw(notifies->FileName, notifies->FileNameLength)));
                        }
                        else
                        {
                            changes.emplace_back(ChangeEntry(ChangeEntry::ChangeType::rename, 0,
                                                             std::move(renameOldNames.top()),
                                                             getPathFromRaw(notifies->FileName, notifies->FileNameLength),
                                                             false));
                            renameOldNames.pop();
                        }

                        break;
                }

                buffer += notifies->NextEntryOffset;
            }
            while (notifies->NextEntryOffset != 0);
        }
    }


    void updateFilesList()
    {
        for (auto &change : changes)
        {
            switch (change.getType())
            {
                case ChangeEntry::ChangeType::add:
                    change.fileIndex = files.size();
                    files.emplace_back(change.getCurrentPath());                    
                    break;
                case ChangeEntry::ChangeType::remove:
                    change.fileIndex = getFileIndex(change.getOldPath());
                    files.erase(change.getOldPath());
                    break;
                case ChangeEntry::ChangeType::rename:
                {
                    const auto iter = files.find(change.getOldPath());
                    if (iter != files.cend())
                    {
                        change.fileIndex = std::distance(files.cbegin(), iter);
                        files.assignElement(iter, change.getCurrentPath());
                    }
                    break;
                }
                case ChangeEntry::ChangeType::modify:
                    change.fileIndex = getFileIndex(change.getCurrentPath());
                    break;
            }
        }
    }


    ChangeEntry::IndexType getFileIndex(const filesystem::Path &path)
    {
        const auto iter = files.find(path);
        if (iter == files.cend())
        {
            throw std::out_of_range("Nonexistent path");
        }

        return std::distance(files.cbegin(), iter);
    }


    void notify()
    {
        parent.handler(changes.cbegin(), changes.cend());
    }
};  // class DirectoryWatcher::Impl


DirectoryWatcher::DirectoryWatcher(const filesystem::Path &path)
    : pImpl(std::make_unique<Impl>(*this, path))
{
}

DirectoryWatcher::DirectoryWatcher(filesystem::Path &&path)
    : pImpl(std::make_unique<Impl>(*this, std::move(path)))
{
}

DirectoryWatcher::~DirectoryWatcher()
{
    stopWatch();
}


void DirectoryWatcher::startWatch()
{
    pImpl->startWatch();
}

void DirectoryWatcher::stopWatch()
{
    pImpl->stopWatch();
}


const filesystem::Path& DirectoryWatcher::getPath() const
{
    return pImpl->getPath();
}

#else   //#ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  //#ifdef _WIN32
