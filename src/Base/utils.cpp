#include "stdafx.h"

#include "CException.hpp"
#include "utils.hpp"

namespace utils {

bool ParamParser::GetBool() const
{
    std::wstring str = *this;
    utils::to_lower(str);

    if (str == L"true" || str == L"yes" || str == L"on")
    {
        return true;
    }

    if (str == L"false" || str == L"no" || str == L"off")
    {
        return false;
    }

    ERROR_E;
    return false;
}

int ParamParser::GetInt() const
{
    if (this->empty())
    {
        return 0;
    }

    int value = 0;
    bool sign = false;

    // the original algorithm skips all the non-digit chars and treats
    // '-' at any position as a number sign.
    // needless to say - it's a bullshit, but other code expects it
    // to work like this, so...
    for (const auto sym : *this)
    {
        wchar ch = sym - '0';
        if (ch < 10)
        {
            value = value * 10 + ch;
        }

        if (sym == '-')
        {
            sign = true;
        }

    }

    return sign ? -value : value;
}

uint32_t ParamParser::GetDword() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar* tstr = c_str();

    uint32_t zn = 0;
    wchar ch;
    for (int i = 0; i < tlen; i++) {
        ch = tstr[i] - '0';
        if (ch < 10)
            zn = zn * 10 + ch;
    }

    return zn;
}

double ParamParser::GetDouble() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar* tstr = c_str();

    int i;
    double zn = 0.0;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9')
            zn = zn * 10.0 + (double)(ch - L'0');
        else if (ch == L'.')
            break;
    }
    i++;
    double tra = 10.0;
    for (; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9') {
            zn = zn + ((double)(ch - L'0')) / tra;
            tra *= 10.0;
        }
    }
    for (i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

uint32_t ParamParser::GetHexUnsigned(void) const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar* tstr = c_str();

    uint32_t zn = 0;
    int i;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];

        ch -= '0';
        if (ch > 9)
            ch = (tstr[i] & (~32)) - ('A' - 10);
        zn = (zn << 4) + ch;
    }
    return zn;
}

bool ParamParser::IsOnlyInt() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar* tstr = c_str();

    for (int i = 0; i < tlen; i++)
        if (tstr[i] < L'0' || tstr[i] > L'9' && tstr[i] != L'-')
            return 0;
    return 1;
}

int ParamParser::GetSmePar(int np, const wchar* ogsim) const {
    int lenogsim = std::wcslen(ogsim);
    int tlen = length();
    // if(tlen<1 || lenogsim<1 || np<0) ERROR_OK("Data in CWStr::GetSmePar()");
    //  if((tlen<1 || lenogsim<1 || np<0))
    //        ASSERT(1);
    ASSERT(!(tlen < 1 || lenogsim < 1 || np < 0));
    int smepar = 0;
    int tekpar = 0;

    const wchar* tstr = c_str();

    if (np > 0) {
        int i;
        for (i = 0; i < tlen; i++) {
            int u;
            for (u = 0; u < lenogsim; u++)
                if (tstr[i] == ogsim[u])
                    break;
            if (u < lenogsim) {
                tekpar++;
                smepar = i + 1;
                if (tekpar == np)
                    break;
            }
        }
        if (i == tlen) {
            ERROR_E;
        }
    }

    return smepar;
}

int ParamParser::GetLenPar(int smepar, const wchar* ogsim) const {
    int i;
    int tlen = length();
    int lenogsim = std::wcslen(ogsim);
    if (tlen < 1 || lenogsim < 1 || smepar > tlen)
        ERROR_E;

    const wchar* tstr = c_str();

    for (i = smepar; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            break;
    }
    return i - smepar;
}

ParamParser ParamParser::GetStrPar(int nps, int npe, const wchar* ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    return std::wstring(c_str() + sme1, sme2 - sme1);
}

int ParamParser::GetCountPar(const wchar* ogsim) const {
    int tlen = length();
    if (tlen < 1)
        return 0;

    int c = 1;
    int lenogsim = std::wcslen(ogsim);
    if (lenogsim < 1)
        return 0;

    const wchar* tstr = c_str();

    for (int i = 0; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            c++;
    }
    return c;
}

} // namespace utils