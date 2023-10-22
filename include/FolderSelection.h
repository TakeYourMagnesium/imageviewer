#pragma once
#include <string>
#include <map>

class PathSelectionSingleton {
public:
    static PathSelectionSingleton& getInstance();
    std::string getPath(const std::string& id) const;
    void setPath(const std::string& id, const std::string& path);
    std::string selectFolder();
    std::string selectFile();

private:
    PathSelectionSingleton();  // Private constructor
    std::map<std::string, std::string> paths;
};