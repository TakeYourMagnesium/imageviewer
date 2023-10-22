#pragma once
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <set>
std::set<std::string> getImagePaths(const std::string& folderPath);
SDL_Texture* loadImage(const std::string &filePath, SDL_Renderer *renderer);
std::vector<std::string> searchTags(const std::string& query, const std::unordered_map<std::string, std::vector<std::string>>& tagMap);
std::unordered_map<std::string, std::vector<std::string>> loadTagMap(const std::string& filename);
std::string getExePath();