#include "utils.h"

namespace utils {
	void cout(LPCSTR str) {
		OutputDebugStringA(str);
	}

	void cout(LPCWSTR wstr) {
		OutputDebugStringW(wstr);
	}

	void coutln(LPCSTR str) {
		cout(str);
		OutputDebugString(TEXT("\n"));
	}

	void coutln(LPCWSTR wstr) {
		cout(wstr);
		OutputDebugString(TEXT("\n"));
	}
}
