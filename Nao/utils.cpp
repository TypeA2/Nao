#include "utils.h"

#include "frameworks.h"

#include <iomanip>

namespace utils {
    inline namespace logging {
        void cout(const char* str) {
            OutputDebugStringW(utf16(str).c_str());
        }

        void cout(const wchar_t* wstr) {
            OutputDebugStringW(wstr);
        }
    }

    inline namespace formatting {
        std::string bytes(int64_t n) {
            std::stringstream ss;
            ss << std::setprecision(3) << std::fixed;
            if (n > 0x1000000000000000) {
                ss << ((n >> 50) / 1024.L) << " EiB";
            } else if (n > 0x4000000000000) {
                ss << ((n >> 40) / 1024.L) << " PiB";
            } else if (n > 0x10000000000) {
                ss << ((n >> 30) / 1024.L) << " TiB";
            } else if (n > 0x40000000) {
                ss << ((n >> 20) / 1024.L) << " GiB";
            } else if (n > 0x100000) {
                ss << ((n >> 10) / 1024.L) << " MiB";
            } else if (n > 0x400) {
                ss << (n / 1024.L) << " KiB";;
            } else {
                ss << n << " bytes";
            }

            // Remove trailing whitespace and zeroes
            std::string s = ss.str();
            std::string suffix = s.substr(s.find_last_of(' '));;
            s = s.substr(0, s.find_last_of(L' '));

            s.erase(s.find_last_not_of("0.") + 1);

            return s + suffix;
        }

        std::wstring wbytes(int64_t n) {
            return utf16(bytes(n));
        }

        std::string bits(int64_t n) {
            std::stringstream ss;
            ss << std::setprecision(3) << std::fixed;
            if (n > 0x1000000000000000) {
                ss << ((n >> 50) / 1024.L) << " Eibit";
            } else if (n > 0x4000000000000) {
                ss << ((n >> 40) / 1024.L) << " Pibit";
            } else if (n > 0x10000000000) {
                ss << ((n >> 30) / 1024.L) << " Tibit";
            } else if (n > 0x40000000) {
                ss << ((n >> 20) / 1024.L) << " Gibit";
            } else if (n > 0x100000) {
                ss << ((n >> 10) / 1024.L) << " Mibit";
            } else if (n > 0x400) {
                ss << (n / 1024.L) << " Kibit";;
            } else {
                ss << n << " bits";
            }

            // Remove trailing whitespace and zeroes
            std::string s = ss.str();
            std::string suffix = s.substr(s.find_last_of(' '));;
            s = s.substr(0, s.find_last_of(L' '));

            s.erase(s.find_last_not_of("0.") + 1);

            return s + suffix;
        }

        std::string perc(double p) {
            return std::to_string(int64_t(round(p * 100.))) + '%';
        }

        std::wstring wperc(double p) {
            return std::to_wstring(int64_t(round(p * 100.))) + L'%';
        }

        std::string utf8(const std::wstring& str) {
            int size = WideCharToMultiByte(CP_UTF8,
                WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS, str.c_str(), int(str.size()),
                nullptr, 0, nullptr, nullptr);

            std::string conv(size, '\0');
            WideCharToMultiByte(CP_UTF8,
                WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS, str.c_str(), -1,
                conv.data(), size, nullptr, nullptr);

            return conv;
        }

        std::wstring utf16(const std::string& str) {
            int size = MultiByteToWideChar(CP_UTF8,
                MB_ERR_INVALID_CHARS, str.c_str(), int(str.size()),
                nullptr, 0);


            std::wstring conv(size, L'\0');
            MultiByteToWideChar(CP_UTF8,
                MB_ERR_INVALID_CHARS, str.c_str(), -1,
                conv.data(), size);

            return conv;
        }

        std::string format_hours(std::chrono::nanoseconds ns, bool ms) {
            std::stringstream ss;
            ss.fill('0');

            auto hours = std::chrono::duration_cast<std::chrono::hours>(ns);
            ns -= hours;
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ns);
            ns -= minutes;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ns);


            ss << hours.count() << ':'
                << std::setw(2)
                << minutes.count() << ':'
                << seconds.count();

            if (ms) {
                ns -= seconds;
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(ns);

                ss << '.' << std::setw(3) << milliseconds.count();
            }

            return ss.str();
        }

        std::string format_minutes(std::chrono::nanoseconds ns, bool ms) {
            std::stringstream ss;
            ss.fill('0');

            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ns);
            ns -= minutes;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ns);
            ss << minutes.count() << ':'
                << std::setw(2) << seconds.count();

            if (ms) {
                ns -= seconds;
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(ns);

                ss << '.' << std::setw(3) << milliseconds.count();
            }

            return ss.str();
        }
    }
}

rectangle dimensions::rect() const {
    return {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height
    };
}

