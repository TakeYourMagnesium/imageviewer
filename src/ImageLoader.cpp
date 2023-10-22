#include "ImageLoader.h"
#include "FolderSelection.h"
#include <filesystem>
#include <set>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include "helpers.h"
#include <iostream>
#include <windows.h>
using json = nlohmann::json;


// Supported image extensions (sets are unique/sorted vectors are dynamic arrays)
const std::set<std::string> supportedExtensions = {".jpg", ".png", ".bmp", ".gif"};

// Get current image Paths
std::set<std::string> getImagePaths(const std::string& folderPath) {
    std::set<std::string> imagePaths;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        auto ext = entry.path().extension().string();

        // Checks if the extension reaches the end with the past the end iterator which is the same for .find/.end
        if (supportedExtensions.find(ext) != supportedExtensions.end()) {
            imagePaths.insert(entry.path().string());
        }
    }
    return imagePaths;
}


// Loads images into ram as an SDL texture
SDL_Texture* loadImage(const std::string &filePath, SDL_Renderer *renderer) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, filePath.c_str());
    if (texture == nullptr) {
        throw std::runtime_error("IMG_LoadTexture Error: " + std::string(IMG_GetError()));
    }
    return texture;
}

// Loads the .json tag map into a hash map in ram
std::unordered_map<std::string, std::vector<std::string>> loadTagMap(const std::string& filename) {
    std::cout << "Json_data tags loading" << std::endl;
    std::ifstream file(filename);
    json json_data;
    
    json_data = json::parse(file);
    std::cout << "Json_data tags loaded, size = "<< json_data.size() << std::endl;


    // Unpacks key/value json data and stores the list of filenames to the tag
    std::unordered_map<std::string, std::vector<std::string>> tagMap;
    for (auto& [key, value] : json_data.items()) {
        tagMap[key] = value.get<std::vector<std::string>>();
    }
    return tagMap;
}
/*std::unordered_map<std::string, std::vector<std::string>> loadTagMap(const std::string& filename) {
    //std::cout << "attempting to open "<<filename << std::endl;
    std::ifstream file(filename);
    //std::cout<< "opened file"<<std::endl;
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            std::cout << line << std::endl;
        }
        file.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }
    json json_data;

    try {
      file >> json_data;
      //std::cout << "Json_data tags loaded, size = "<< json_data.size() << std::endl;

      //Unpacks key/value json data and stores the list of filenames to the tag
      std::unordered_map<std::string, std::vector<std::string>> tagMap;

      for (auto& [key, value] : json_data.items()) {
          tagMap[key] = value.get<std::vector<std::string>>();
    }
    return tagMap;
    } catch (char const *e) {
        std::cerr << "Exception: "<<e<< std::endl;
        std::cout << "Press any key to exit...";
        getchar(); // Wait for user to press any key
        exit(1); // Or return an appropriate error code
    }
    catch(...){
       std::cerr << "Exception, press any key to exit "<<std::endl;
       getchar();
       exit(2);
    }
}*/


// Search through tags in tagMap from search input
std::vector<std::string> searchTags(const std::string& query, const std::unordered_map<std::string, std::vector<std::string>>& tagMap) {
    std::vector<std::string> intersectedPaths;
    std::unordered_map<std::string, int> filenameCounts;
    // Split the query into individual tags (assuming they are comma-separated)
    std::istringstream iss(query);
    std::vector<std::string> tags;
    std::string tag;
    while (std::getline(iss, tag, ',')) {
        tag = trim(tag);
        if (tag != "") {
            tags.push_back(tag);
        }
    }

    // Loop through the tagMap and count the occurrences of each filename for each tag in the query
    for (const auto& queryTag : tags) {
        auto it = tagMap.find(queryTag);
        if (it != tagMap.end()) { // If this tag is in the tagMap
            const auto& filenames = it->second;
            for (const auto& filename : filenames) {
                ++filenameCounts[filename];
            }
        }
    }
    
    // Find filenames that appeared as many times as there are tags in the query
    for (const auto& [filename, count] : filenameCounts) {
        if (count == tags.size()) {
            std::string fullPath = PathSelectionSingleton::getInstance().getPath("imageFolder") + "\\" + filename;            
            intersectedPaths.push_back(fullPath);
        }
    }

    return intersectedPaths;
}

std::string getExePath() {
    char buffer[MAX_PATH];  // MAX_PATH is a constant defined in windows.h
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}