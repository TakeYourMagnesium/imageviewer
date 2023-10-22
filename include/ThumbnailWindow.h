#pragma once  // Use this or #ifndef/#define/#endif guards to prevent double inclusion

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <unordered_map>
#include <string>
#include <set>

class Thumbnail {
public:
    int thumbWidth;
    int thumbHeight;
    int thumbSpacing;
    SDL_Window* thumbnailWindow;
    SDL_Renderer* thumbnailRenderer;
    SDL_Renderer* IMGRenderer;
    std::unordered_map<std::string, SDL_Texture*>& preloadedTexturesIMG;
    std::unordered_map<std::string, SDL_Texture*>& preloadedTexturesTHUMB;
    const std::unordered_map<std::string, std::vector<std::string>>& tagMap;
    std::vector<SDL_Texture*> filteredTextures;
    std::set<std::string> imgPaths;
    std::vector<std::string> filteredPaths;
    std::set<std::string>& loadedTexturesPath;
    std::vector<std::string>& thumbnailOrder;
    // Constructor to initialize the thumbnail dimensions
    Thumbnail(int thumbWidth, int thumbHeight, int thumbSpacing, SDL_Window* thumbnailWindow, SDL_Renderer* thumbnailRenderer, std::unordered_map<std::string, SDL_Texture*>& initpreloadedTexturesTHUMB,const std::unordered_map<std::string, std::vector<std::string>>& tagMap,std::set<std::string> imgPaths, std::set<std::string>& initloadedTexturesPath, SDL_Renderer* IMGRenderer,std::unordered_map<std::string, SDL_Texture*>& initpreloadedTexturesIMG, std::vector<std::string>& initthumbnailOrder);

    

    // Method to render the search bar
    void renderSearchBar(bool& needsImGuiUpdate);

    // Method to render the thumbnail window
    void renderThumbnailWindow(std::string& currentImage,
                               bool& needsRenderUpdate, bool& needsThumbLoadUpdate);

    void renderButtons(const std::string& path, SDL_Texture* texture);
    bool handleButtonClick(const std::string& path, std::string& initcurrentImage);
    void lazyLoadTextures();
    void loadMore();
};