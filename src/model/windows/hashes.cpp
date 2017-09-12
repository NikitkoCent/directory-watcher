#ifdef _WIN32

#include "../path.h"
#include <cwctype>  // std::towlower

namespace std
{
    std::size_t hash<filesystem::Path::string_type>::operator()(const filesystem::Path::string_type &src) const
    {
        const hash<filesystem::Path::char_type> baseHasher;
        std::size_t result = 0;

        for (const filesystem::Path::char_type &c : src)
        {
            result ^= baseHasher(std::towlower(c)) + 0x9e3779b9 + (result << 6) + (result >> 2);
        }

        return result;
    }


    std::size_t hash<filesystem::Path>::operator()(const filesystem::Path &src) const
    {
        return hash<filesystem::Path::string_type>()(src.getPathString());
    }
}

#else   // #ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  // #ifdef _WIN32
