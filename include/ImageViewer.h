#pragma once
#include <SDL.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <set>

enum ClickSectionArea {
    NONE,
    LEFT,
    RIGHT
};

class ImageViewer {
public:
    std::set<std::string>& loadedTexturesPath;
    std::unordered_map<std::string, SDL_Texture*>& preloadedTextures;
    ImageViewer(SDL_Window* window, SDL_Renderer* initrenderer, std::unordered_map<std::string, SDL_Texture*>& textures, std::string& currentImage, std::set<std::string>& initloadedTexturesPath, std::vector<std::string>& thumbnailOrder);
    void handleEvents(SDL_Event& e, bool& needsRenderUpdate, bool& needsImGuiUpdate, bool& windowResized);
    void renderImage();
    std::string& currentImage;
    std::vector<std::string>& thumbnailOrder;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    ClickSectionArea clickSection;
    SDL_Rect calculateDestRect();
};