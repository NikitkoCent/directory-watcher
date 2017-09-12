#include "directory_watcher.h"


DirectoryWatcher::ChangeEntry::ChangeType DirectoryWatcher::ChangeEntry::getType() const
{
    return changeType;
}


DirectoryWatcher::ChangeEntry::IndexType DirectoryWatcher::ChangeEntry::getFileIndex() const
{
    return fileIndex;
}


bool DirectoryWatcher::ChangeEntry::isRoot() const
{
    return root;
}


const filesystem::Path& DirectoryWatcher::ChangeEntry::getOldPath() const
{
    return oldPath;
}

const filesystem::Path& DirectoryWatcher::ChangeEntry::getCurrentPath() const
{
    return currentPath;
}
