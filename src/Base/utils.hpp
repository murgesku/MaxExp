#pragma once

#include <string>
#include <cstdio>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <cwchar>

namespace utils {

class ParamParser : public std::wstring
{
public:
    ParamParser(const std::wstring& str)
        : std::wstring{ str }
    {
    }

    ParamParser() = default;
    ~ParamParser() = default;

    bool GetBool() const;
    int GetInt(void) const;
    uint32_t GetDword(void) const;
    double GetDouble(void) const;
    uint32_t GetHexUnsigned(void) const;

    bool IsOnlyInt(void) const;

    // Функции для работы с параметрами
    // Примеры :
    //      Str="count=5,7"    GetCountPar("=,")           return 3
    //      Str="count=5,7"    GetStrPar(str,1,"=")        str="5,7"
    //      Str="count=5,7"    GetStrPar(2,"=,").GetInt()  return 7

    int GetSmePar(int np, const wchar* ogsim) const;
    int GetLenPar(int smepar, const wchar* ogsim) const;

    int GetCountPar(const wchar* ogsim) const;
    ParamParser GetStrPar(int np, const wchar* ogsim) const {
        int sme = GetSmePar(np, ogsim);
        return std::wstring(c_str() + sme, GetLenPar(sme, ogsim));
    }

    ParamParser GetStrPar(int nps, int npe, const wchar* ogsim) const;
};

template<typename... Args>
inline std::string format(const char* format, Args... args)
{
    // TODO: replace with std::format/fmt::format
    char buf[10240];
    if (std::snprintf(buf, sizeof(buf) / sizeof(char), format, args...) < 0)
    {
        throw std::runtime_error("snprintf() failed");
    }
    return std::string{buf};
}

template<typename... Args>
inline std::wstring format(const wchar_t* format, Args... args)
{
    // TODO: replace with std::format/fmt::format
    wchar_t buf[10240];
    if (std::swprintf(buf, sizeof(buf) / sizeof(wchar_t), format, args...) < 0)
    {
        throw std::runtime_error("swprintf() failed");
    }
    return std::wstring{buf};
}

inline std::string from_wstring(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

inline std::wstring to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

// TODO: replace with string::starts_with from C++20
inline bool starts_with(const std::wstring& src, const std::wstring& sub)
{
    return (src.find(sub) == 0);
}

inline void to_lower(std::wstring& str, size_t offset = 0)
{
    for (size_t i = offset; i < str.length(); ++i)
    {
        str[i] = towlower(str[i]);
    }
}

inline std::wstring trim(const std::wstring& str)
{
    std::wstring tmp{str};
    tmp.erase(0, tmp.find_first_not_of(L" \t\r\n"));
    tmp.erase(tmp.find_last_not_of(L" \t\r\n") + 1);
    return tmp;
}

inline std::wstring& replace(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
    size_t tlen = str.length();
    if (tlen < 1 || tlen < from.length())
    {
        return str;
    }

    size_t offset = 0;
    while (true)
    {
        offset = str.find(from, offset);
        if (offset == std::wstring::npos)
        {
            break;
        }
        str.replace(offset, from.length(), to);
        offset += to.length();
    }

    return str;
}

} // namespace utils