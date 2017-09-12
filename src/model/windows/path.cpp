#ifdef _WIN32

#include "../path.h"
#include <utility>      // std::move
#include <algorithm>    // std::replace

namespace filesystem
{
    namespace
    {
        constexpr auto dirDelLen = 1;
        constexpr auto driveDelLen = 2;
        constexpr wchar_t dirDelimiter[dirDelLen + 1] = L"\\";
        constexpr wchar_t driveDelimiter[driveDelLen + 1] = L":\\";
    }

    Path::Path() = default;

    Path::Path(const string_type &pathString)
        : path(pathString)
    {
        std::replace(path.begin(), path.end(), L'/', dirDelimiter[0]);
    }

    Path::Path(string_type &&pathString)
        : path(std::move(pathString))
    {
        std::replace(path.begin(), path.end(), L'/', dirDelimiter[0]);
    }

    Path::Path(const char_type *pathString)
        : path(pathString)
    {
        std::replace(path.begin(), path.end(), L'/', dirDelimiter[0]);
    }


    const Path::string_type& Path::getPathString() const
    {
        return path;
    }


    Path Path::getRelativePath() const
    {
        const auto pos = path.find(driveDelimiter);
        return std::move(path.substr((pos == path.npos ? 0 : pos + driveDelLen)));
    }


    Path Path::getFilename() const
    {
        if (path.empty())
        {
            return Path();
        }

        const auto pos = path.find_last_of(dirDelimiter);
        if (pos == path.length() - 1)
        {
            return Path(L".");
        }

        return std::move(path.substr((pos == path.npos ? 0 : pos + dirDelLen)));
    }


    Path& Path::operator/=(const Path &right)
    {
        if (path.empty())
        {
            path = right.path;
        }
        else
        {
            path.reserve(path.length() + right.path.length() + 2);    // + 2 for dirDelimiter and '\0'

            if (path.back() != dirDelimiter[0])
            {
                path.push_back(dirDelimiter[0]);
            }
            if (!right.path.empty())
            {
                if (right.path.front() != dirDelimiter[0])
                {
                    path += right.path;
                }
                else
                {
                    path.append(right.path.cbegin() + 1, right.path.cend());
                }
            }
        }

        return *this;
    }


    bool operator==(const Path &left, const Path &right)
    {
        return left.getPathString() == right.getPathString();
    }

    bool operator!=(const Path &left, const Path &right)
    {
        return !(left == right);
    }


    bool operator<(const Path &left, const Path &right)
    {
        return left.getPathString() < right.getPathString();
    }

    bool operator>(const Path &left, const Path &right)
    {
        return right < left;
    }


    bool operator<=(const Path &left, const Path &right)
    {
        return !(left > right);
    }

    bool operator>=(const Path &left, const Path &right)
    {
        return !(left < right);
    }


    Path operator/(const Path &left, const Path &right)
    {
        return Path(left) /= right;
    }
}

#else   // #ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  // #ifdef _WIN32
