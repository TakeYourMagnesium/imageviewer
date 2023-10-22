#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include "ThumbnailWindow.h"
#include <string>
#include "ImageLoader.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <set>
#include <iostream>

Thumbnail::Thumbnail(int thumbWidth, int thumbHeight, int thumbSpacing, SDL_Window* thumbnailWindow, SDL_Renderer* thumbnailRenderer, std::unordered_map<std::string, SDL_Texture*>& initpreloadedTexturesTHUMB,const std::unordered_map<std::string, std::vector<std::string>>& tagMap,std::set<std::string> imgPaths, std::set<std::string>& initloadedTexturesPath, SDL_Renderer* IMGRenderer,std::unordered_map<std::string, SDL_Texture*>& initpreloadedTexturesIMG, std::vector<std::string>& initthumbnailOrder)
            : thumbWidth(thumbWidth), thumbHeight(thumbHeight), thumbSpacing(thumbSpacing), thumbnailWindow(thumbnailWindow), thumbnailRenderer(thumbnailRenderer), preloadedTexturesTHUMB(initpreloadedTexturesTHUMB), tagMap(tagMap), filteredTextures(), imgPaths(imgPaths), loadedTexturesPath(initloadedTexturesPath), preloadedTexturesIMG(initpreloadedTexturesIMG), IMGRenderer(IMGRenderer), thumbnailOrder(initthumbnailOrder) {
                            //std::cout << "thumbnail loadedTexturesPath constructor:  " << &loadedTexturesPath << std::endl;

            }

    void Thumbnail::renderSearchBar(bool& needsImGuiUpdate) {
        int width, height;
        SDL_GetWindowSize(thumbnailWindow, &width, &height);
        ImGui::SetNextWindowSize(ImVec2(width, 40));
        ImGui::SetNextWindowPos(ImVec2(0, height - 40));
        ImGui::Begin("SearchBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        static char searchBuffer[256];
        static bool searched = false;
        if (ImGui::InputText("##SearchBox", searchBuffer, sizeof(searchBuffer))) {
            needsImGuiUpdate = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            searched = true;
        }
        if (searched) {
            filteredPaths.clear();
            filteredPaths = searchTags(searchBuffer, tagMap);
            lazyLoadTextures();  // Lazy-load the first 25 textures for the filtered paths
            searched = false;
        }
        ImGui::End();
    }

    // Render the thumbnail window
    void Thumbnail::renderThumbnailWindow(std::string& currentImage,
                           bool& needsRenderUpdate, bool& needsThumbLoadUpdate) {
            int width, height;
            SDL_GetWindowSize(thumbnailWindow, &width, &height);
            ImGui::SetNextWindowSize(ImVec2(width, height-30));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Thumbnails", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

            
            int numColumns = width / (thumbWidth + thumbSpacing);  // Calculate number of columns based on window width
            int countColumn = 0;
            int countThumb = 0;
             for (const auto& path : thumbnailOrder) {
                SDL_Texture* texture = preloadedTexturesTHUMB[path];  // Fetch the SDL_Texture* from the map
                renderButtons(path, texture);  // This function now uses a string as an identifier and an SDL_Texture*
                if (handleButtonClick(path, currentImage)) {
                    needsRenderUpdate = true;
                }
                // Manage the grid layout
                if (++countColumn < numColumns) {
                    ImGui::SameLine();
                } else {
                    countColumn = 0;
                }
                if (++countThumb >= preloadedTexturesTHUMB.size()){
                    if (ImGui::Button("Load 25 More")){
                        loadMore();
                        needsThumbLoadUpdate= true;
                    }
                    break;
                }
            }

        ImGui::End();
    }       

    void Thumbnail::renderButtons(const std::string& path, SDL_Texture* texture) {
        ImGui::PushID(path.c_str());  // Make sure each button has a unique ID based on the path
        ImGui::ImageButton((void*)texture, ImVec2(thumbWidth, thumbHeight));
        ImGui::PopID();
    }

    bool Thumbnail::handleButtonClick(const std::string& path, std::string& currentImage) {
    if (ImGui::IsItemClicked()) {
        
        currentImage = path;  // currentImage could be changed to a string representing the current path
        return true;
    }
    return false;
}


void Thumbnail::lazyLoadTextures() {
     for (const auto& pair : preloadedTexturesTHUMB) {
        SDL_DestroyTexture(pair.second);
    }
    for (const auto& pair : preloadedTexturesIMG) {
        SDL_DestroyTexture(pair.second);
    }
    thumbnailOrder.clear();
    preloadedTexturesTHUMB.clear();
    preloadedTexturesIMG.clear();
    loadedTexturesPath.clear();
    int count = 0;
    for (const auto& path : filteredPaths) {
        if (count >= 25) break;

        // If the texture path is not in loadedTexturesPath
        if (loadedTexturesPath.find(path) == loadedTexturesPath.end()) {
            
            // Load texture if not already loaded
            preloadedTexturesTHUMB[path] = loadImage(path, thumbnailRenderer);
            preloadedTexturesIMG[path] = loadImage(path, IMGRenderer);
            //std::cout << "thumbnail loadedTexturesPath Lazy Load:  " << &loadedTexturesPath << std::endl;
            {
                /* code */
            }
            
            // Add path to the set of loaded textures
            loadedTexturesPath.insert(path);
            thumbnailOrder.push_back(path);
            // Increment the counter for textures loaded into RAM
            ++count;
        }
    }

    // Here you could also implement logic to remove least-recently-used textures
    // if the size of loadedTexturesPath becomes too large.
}


void Thumbnail::loadMore() {
    int count = 0;
    size_t startAt = thumbnailOrder.size();
    for (size_t i = startAt; i < filteredPaths.size(); ++i) {
        if (count >= 25) break;

        // If the texture path is not in loadedTexturesPath
        if (loadedTexturesPath.find(filteredPaths[i]) == loadedTexturesPath.end()) {
            
            // Load texture if not already loaded
            preloadedTexturesTHUMB[filteredPaths[i]] = loadImage(filteredPaths[i], thumbnailRenderer);
            preloadedTexturesIMG[filteredPaths[i]] = loadImage(filteredPaths[i], IMGRenderer);
            //std::cout << "thumbnail loadedTexturesPath Lazy Load:  " << &loadedTexturesPath << std::endl;
            {
                /* code */
            }
            
            // Add path to the set of loaded textures
            loadedTexturesPath.insert(filteredPaths[i]);
            thumbnailOrder.push_back(filteredPaths[i]);
            // Increment the counter for textures loaded into RAM
            ++count;
        }
    }

    // Here you could also implement logic to remove least-recently-used textures
    // if the size of loadedTexturesPath becomes too large.
}