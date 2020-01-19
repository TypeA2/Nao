#include "com_thread.h"
#include "utils.h"

com_thread::com_thread(DWORD init) {
    HRESULT res = CoInitializeEx(nullptr, init);
    ASSERT(res == S_OK);
}

com_thread::~com_thread() {
    CoUninitialize();
}

