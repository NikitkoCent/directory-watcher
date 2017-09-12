#ifndef DIRECTORY_WATCHER_H
#define DIRECTORY_WATCHER_H

#include "path.h"
#include <functional>   // std::function
#include <utility>      // std::forward
#include <memory>       // std::unique_ptr
#include <vector>       // ChangeContainer typedef
#include <cstdint>      // std::uint64_t (IndexType typedef)


/* DirectoryWatcher provides base support for directory changes monitoring
 * Intended for single use only (call startWatch after shutdown by stopWatch is undefined behaviour
 * This class requires a separate thread for work (see startWatch method) */
class DirectoryWatcher
{
public:
    class ChangeEntry;

private:
    class Impl;
    using ChangeContainer = std::vector<ChangeEntry>;

public:
    using ChangeIterator = ChangeContainer::const_iterator;     // implementation defined

    /* ChangeEntry represents any change in directory */
    class ChangeEntry
    {
    public:
        /*
         * add - any file added to directory
         * remove - any file removed from directory
         * rename - any file renamed
         * modify - any file changed
         */
        enum class ChangeType{ add, remove, rename, modify };
        using IndexType = std::uint64_t;


        ChangeEntry(const ChangeEntry &src) = default;
        ChangeEntry(ChangeEntry &&src) = default;

        ChangeEntry& operator=(const ChangeEntry &right) = default;
        ChangeEntry& operator=(ChangeEntry &&right) = default;

        ~ChangeEntry() = default;        

        /* returns type of this change */
        ChangeType getType() const;

        /* returns number of changed file
         * Note : all files in directory are numbered in the "detection" order (i.e. in changes arrive order)
         */
        IndexType getFileIndex() const;

        /* returns true if this change is about tracked directory itself; false otherwise
         * Note : on Windows, this function always returns false */
        bool isRoot() const;

        /*
         * returns old path (relative to tracked directory) of changed file
         * if getType() == ChangeType::add then returns Path()
         * if getType() == ChangeType::remove then returns path of deleted file
         * if getType() == ChangeType::rename then returns old path of renamed file
         * if getType() == ChangeType::modify then returns Path()
         */
        const filesystem::Path& getOldPath() const;

        /*
         * returns current path (relative to tracked directory) of changed file
         * if getType() == ChangeType::add then returns path of added file
         * if getType() == ChangeType::remove then returns Path()
         * if getType() == ChangeType::rename then returns new path of renamed file
         * if getType() == ChangeType::modify then returns path of this file
         */
        const filesystem::Path& getCurrentPath() const;

    private:
        friend class DirectoryWatcher;
        friend class DirectoryWatcher::Impl;

        ChangeType changeType;
        IndexType fileIndex;
        filesystem::Path oldPath;
        filesystem::Path currentPath;
        bool root;

        template<typename OPath, typename CPath>
        ChangeEntry(ChangeType type, IndexType fileIndex,
                    OPath &&oldPath, CPath &&currentPath,
                    bool isRoot)
            : changeType(type), fileIndex(fileIndex),
              oldPath(std::forward<OPath>(oldPath)), currentPath(std::forward<CPath>(currentPath)),
              root(isRoot)
        {
        }
    };


    /*
     * Constructs DirectoryWatcher object
     *
     * Parameters:
     *  path                -   full path to tracked directory
     *
     * Throws:
     *  std::system_error   -   directory processing with this path causes a error
     *                          (for example, the directory does not exists)
     */
    DirectoryWatcher(const filesystem::Path &path);
    DirectoryWatcher(filesystem::Path &&path);    

    ~DirectoryWatcher();

    /*
     * Starts monitoring the directory with path, specified in constructor
     * blocks current thread until call stopWatch()
     * call from current changeHandler is undefined behaviour
     * call from a several threads is undefined behaviour (NO THREAD-SAFETY)
     *
     * Parameters:
     *  Callable &&changeHandler    -   calls when directory change event(s) have been registered
     *      changeHandler signature :
     *          void(ChangeIterator begin, ChangeIterator end)
     *              where begin, end = iterators of range [begin, end) of changes
     *
     * Throws:
     *  std::system_error           -   any system error occured
     */
    template<typename Callable>
    void startWatch(Callable &&changeHandler)
    {
        handler = std::forward<Callable>(changeHandler);
        startWatch();
    }

    /*
     * Stops directory monitoring operation. After this operation, calling startWatch again is undefined behaviour
     *
     * Throws:
     *  std::system_error           -   any system error occured
     */
    void stopWatch();

    /*
     * Returns path specified by constructor
     */
    const filesystem::Path& getPath() const;

private:    
    const std::unique_ptr<Impl> pImpl;
    std::function<void(ChangeIterator, ChangeIterator)> handler;    


    DirectoryWatcher(const DirectoryWatcher&) = delete;
    DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;

    DirectoryWatcher(DirectoryWatcher&&) = delete;
    DirectoryWatcher& operator=(DirectoryWatcher&&) = delete;

    void startWatch();
};


#endif // DIRECTORY_WATCHER_H