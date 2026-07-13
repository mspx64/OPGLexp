#pragma once
#include <codecvt>
#include <commdlg.h>
#include <iostream>
#include <locale>
#include <string>
#include <windows.h>

namespace FileDial {
std::string WideToString(const std::wstring& wstr) {
    if (wstr.empty())
        return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);

    std::string result(size_needed, 0);

    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &result[0], size_needed, NULL, NULL);

    return result;
}

std::string OpenFile() {
    wchar_t       fileName[MAX_PATH] = L""; // Use wchar_t
    OPENFILENAMEW ofn                = {0}; // Use the wide version
    ofn.lStructSize                  = sizeof(ofn);
    ofn.lpstrFilter                  = L"All Files\0*.*\0Text Files\0*.TXT\0"; // L prefix for wide string
    ofn.lpstrFile                    = fileName;
    ofn.nMaxFile                     = MAX_PATH;
    ofn.Flags                        = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        std::wcout << L"Selected file: " << fileName << std::endl;
    } else {
        std::wcout << L"No file selected." << std::endl;
    }
    return WideToString(fileName);
}
} // namespace FileDial
