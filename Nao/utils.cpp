#include "utils.h"

#include <iomanip>

namespace utils {
    void cout(LPCSTR str) {
        OutputDebugStringA(str);
    }

    void cout(LPCWSTR wstr) {
        OutputDebugStringW(wstr);
    }

    void coutln(LPCSTR str) {
        cout(str);
        OutputDebugStringW(L"\n");
    }

    void coutln(LPCWSTR wstr) {
        cout(wstr);
        OutputDebugStringW(L"\n");
    }

    std::string bytes(int64_t n) {
        std::stringstream ss;
        ss << std::setprecision(3) << std::fixed;
        if (n > 0x1000000000000000) {
            ss << ((n >> 50) / 1024.L) << " EiB";;
        } else if (n > 0x4000000000000) {
            ss << ((n >> 40) / 1024.L) << " PiB";;
        } else if (n > 0x10000000000) {
            ss << ((n >> 30) / 1024.L) << " TiB";;
        } else if (n > 0x40000000) {
            ss << ((n >> 20) / 1024.L) << " GiB";;
        } else if (n > 0x100000) {
            ss << ((n >> 10) / 1024.L) << " MiB";
        } else if (n > 0x400) {
            ss << (n / 1024.L) << " KiB";;
        } else {
            ss << n << " bytes";
        }

        return ss.str();
    }

    std::wstring wbytes(int64_t n) {
        std::wstringstream ss;
        ss << std::setprecision(3) << std::fixed;
        if (n > 0x1000000000000000) {
            ss << ((n >> 50) / 1024.L) << L" EiB";;
        } else if (n > 0x4000000000000) {
            ss << ((n >> 40) / 1024.L) << L" PiB";;
        } else if (n > 0x10000000000) {
            ss << ((n >> 30) / 1024.L) << L" TiB";;
        } else if (n > 0x40000000) {
            ss << ((n >> 20) / 1024.L) << L" GiB";;
        } else if (n > 0x100000) {
            ss << ((n >> 10) / 1024.L) << L" MiB";
        } else if (n > 0x400) {
            ss << (n / 1024.L) << L" KiB";;
        } else {
            ss << n << L" bytes";
        }

        return ss.str();
    }

    std::string perc(double p) {
        return std::to_string(int64_t(round(p * 100.))) + '%';
    }

    std::wstring wperc(double p) {
        return std::to_wstring(int64_t(round(p * 100.))) + L'%';
    }

    std::string utf8(const std::wstring& str) {
        size_t required;
        wcstombs_s(&required, nullptr, 0, str.c_str(), _TRUNCATE);

        std::string res(required - 1, '\0');
        wcstombs_s(&required, res.data(), required, str.c_str(), _TRUNCATE);

        return res;
    }

    std::wstring utf16(const std::string& str) {
        size_t required;
        mbstowcs_s(&required, nullptr, 0, str.c_str(), _TRUNCATE);

        std::wstring res(required - 1, L'\0');
        mbstowcs_s(&required, res.data(), required, str.c_str(), _TRUNCATE);

        return res;
    }



}
