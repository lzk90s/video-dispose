#pragma once

#include <string>

std::wstring StringToWString(const std::string &str) {
    std::wstring wstr(str.length(), L' ');
    std::copy(str.begin(), str.end(), wstr.begin());
    return wstr;
}

//只拷贝低字节至string中
std::string WStringToString(const std::wstring &wstr) {
    std::string str(wstr.length(), ' ');
    std::copy(wstr.begin(), wstr.end(), str.begin());
    return str;
}