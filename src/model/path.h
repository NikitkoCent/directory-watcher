#ifndef PATH_H
#define PATH_H

#include <string>
#include <functional>   //std::hash
#include <cstddef>      //std::size_t

namespace filesystem
{
    namespace
    {
#ifdef _WIN32
        using CharType = wchar_t;
#else
        using CharType = char;
#endif
    }


    /* Case-insensetive char traits */
    class CICharTraits
    {
    private:
        using Base = std::char_traits<CharType>;

    public:
        using char_type = typename Base::char_type;
        using int_type = typename Base::int_type;
        using off_type = typename Base::off_type;
        using pos_type = typename Base::pos_type;
        using state_type = typename Base::state_type;

        static void assign(char_type &r, const char_type &a);
        static char_type* assign(char_type *p, std::size_t count, char_type a);

        static bool eq(const char_type &c1, const char_type &c2);
        static bool lt(const char_type &c1, const char_type &c2);

        static char_type* move( char_type* dest, const char_type* src, std::size_t count );

        static char_type* copy( char_type* dest, const char_type* src, std::size_t count );

        static int compare(const char_type *s1, const char_type *s2, std::size_t n);

        static std::size_t length( const char_type* s );

        static const char_type* find(const char_type *s, std::size_t n, const char_type &a);

        static char_type to_char_type( int_type c ) noexcept;

        static int_type to_int_type( char_type c );

        static bool eq_int_type( int_type c1, int_type c2 );

        static int_type eof();

        static int_type not_eof( int_type e ) noexcept;
    };

    /* Class representing filesystem path */
    class Path
    {
    public:
        using char_type = CharType;
        using string_type = std::basic_string<char_type, CICharTraits>;

        Path();
        Path(const string_type &pathString);
        Path(string_type &&pathString);
        Path(const char_type *pathString);

        template<typename InputIterator>
        Path(InputIterator begin, InputIterator end)
            : path(begin, end)
        {
        }        


        const string_type& getPathString() const;        

        Path getRelativePath() const;

        /* Returns all after the last dir-delimiter */
        Path getFilename() const;

        /* Returns "this{dir-delimiter}right" */
        Path& operator/=(const Path &right);

    private:
        string_type path;
    };


    bool operator==(const Path &left, const Path &right);
    bool operator!=(const Path &left, const Path &right);
    bool operator<(const Path &left, const Path &right);
    bool operator>(const Path &left, const Path &right);
    bool operator<=(const Path &left, const Path &right);
    bool operator>=(const Path &left, const Path &right);
    Path operator/(const Path &left, const Path &right);


    template<typename OStream>
    OStream& operator<<(OStream &stream, const Path::string_type &string)
    {
        return stream << string.c_str();
    }

    template<typename OStream>
    OStream& operator<<(OStream &stream, const Path &path)
    {
        return stream << path.getPathString();
    }
}


namespace std
{
    /* std::hash specializations for Path and Path::string_type */

    template<>
    struct hash<filesystem::Path::string_type>
    {
        std::size_t operator()(const filesystem::Path::string_type &src) const;
    };

    template<>
    struct hash<filesystem::Path>
    {
        std::size_t operator()(const filesystem::Path &src) const;
    };
}

#endif // PATH_H
