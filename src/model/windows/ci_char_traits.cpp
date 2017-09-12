#ifdef _WIN32

#include "../path.h"
#include <cwctype>      //std::towlower

namespace filesystem
{
    void CICharTraits::assign(char_type &r, const char_type &a)
    {
        return Base::assign(r, a);
    }

    CICharTraits::char_type* CICharTraits::assign(char_type *p, std::size_t count, char_type a)
    {
        return Base::assign(p, count, a);
    }


    bool CICharTraits::eq(const char_type &c1, const char_type &c2)
    {
        return Base::eq(std::towlower(c1), std::towlower(c2));
    }

    bool CICharTraits::lt(const char_type &c1, const char_type &c2)
    {
        return Base::lt(std::towlower(c1), std::towlower(c2));
    }


    CICharTraits::char_type* CICharTraits::move(char_type *dest, const char_type *src, std::size_t count)
    {
        return Base::move(dest, src, count);
    }

    CICharTraits::char_type* CICharTraits::copy(char_type *dest, const char_type *src, std::size_t count)
    {
        return Base::copy(dest, src, count);
    }


    int CICharTraits::compare(const char_type *s1, const char_type *s2, std::size_t n)
    {
        for (; n > 0; --n)
        {
            const auto c1 = std::towlower(*s1);
            const auto c2 = std::towlower(*s2);

            if (c1 < c2)
            {
                return -1;
            }
            if (c1 > c2)
            {
                return 1;
            }
        }

        return 0;
    }


    std::size_t CICharTraits::length(const char_type *s)
    {
        return Base::length(s);
    }


    const CICharTraits::char_type* CICharTraits::find(const char_type *s, std::size_t n, const char_type &a)
    {
        const auto c = std::towlower(a);
        for (; (n > 0) && (std::towlower(*s) != c); --n);

        return nullptr;
    }


    CICharTraits::char_type CICharTraits::to_char_type(int_type c) noexcept
    {
        return Base::to_char_type(c);
    }

    CICharTraits::int_type CICharTraits::to_int_type(char_type c)
    {
        return Base::to_int_type(c);
    }


    bool CICharTraits::eq_int_type(int_type c1, int_type c2)
    {
        return Base::eq_int_type(c1, c2);
    }


    CICharTraits::int_type CICharTraits::eof()
    {
        return Base::eof();
    }

    CICharTraits::int_type CICharTraits::not_eof(int_type e) noexcept
    {
        return Base::not_eof(e);
    }
}

#else   // #ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  // #ifdef _WIN32
