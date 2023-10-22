#include "FolderSelection.h"
#include <shlobj.h>
#include <stdexcept> // For std::runtime_error
#include <string>
#include <map>
#include <windows.h>
#include <commdlg.h>

std::string PathSelectionSingleton::getPath(const std::string& id) const {
    auto it = paths.find(id);
    if (it != paths.end()) {
        return it->second;
    }
    return "";  // or throw an exception if you prefer
}

void PathSelectionSingleton::setPath(const std::string& id, const std::string& path) {
    paths[id] = path;
}

PathSelectionSingleton& PathSelectionSingleton::getInstance() {
    static PathSelectionSingleton instance;
    return instance;
}

PathSelectionSingleton::PathSelectionSingleton() {
    // Empty constructor body
}

std::string PathSelectionSingleton::selectFolder() {
    std::string tempPath;

    // Initialize BROWSEINFO structure
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select a folder:";

    // Show folder selection dialog
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    // Check the returned ITEMIDLIST
    if (pidl != nullptr) {
        TCHAR path[MAX_PATH];
        
        // Retrieve the path from the ITEMIDLIST
        if (SHGetPathFromIDList(pidl, path)) {
            tempPath = std::string(path);
        } else {
            throw std::runtime_error("Failed to get path from ID list");
        }

        // Free the ITEMIDLIST using COM's IMalloc
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        } else {
            throw std::runtime_error("Failed to get IMalloc interface");
        }
    }

    // Return the selected folder's path
    return tempPath;
}

std::string PathSelectionSingleton::selectFile() {
    std::string filePath;
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "";
    if (GetOpenFileName(&ofn))
        filePath = std::string(fileName);
    return filePath;
}