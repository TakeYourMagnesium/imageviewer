#include <windows.h>
#include <stdio.h>
#include "../include/SDL2/SDL.h"
#include "../include/SDL2/SDL_image.h"
#include <shlobj.h>
#include <string>
#include <filesystem>
#include <vector>

std::string selectFolder() {
    std::string folderPath;
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select a folder:";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            folderPath = std::string(path);
        }

        // Free the item ID list
        IMalloc *imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
    return folderPath;
}

std::vector<std::string> getImagePaths(const std::string& folderPath) {
    std::vector<std::string> imagePaths;
    for (const auto & entry : std::filesystem::directory_iterator(folderPath)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".gif") {
            imagePaths.push_back(entry.path().string());
        }
    }
    return imagePaths;
}

SDL_Texture* loadImage(const std::string &filePath, SDL_Renderer *renderer) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, filePath.c_str());
    if (texture == nullptr) {
        printf("IMG_LoadTexture Error: %s\n", IMG_GetError());
    }
    return texture;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Test Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    std::string folderPath = selectFolder();
    printf("Selected folder: %s\n", folderPath.c_str());

    std::vector<std::string> imagePaths = getImagePaths(folderPath);

    // Print out loaded image paths
    /*for (const auto& path : imagePaths) {
        printf("Loaded image: %s\n", path.c_str());
    }*/

    std::vector<SDL_Texture*> preloadedTextures;
    for (const auto& imagePath : imagePaths) {
        preloadedTextures.push_back(loadImage(imagePath, renderer));
    }


    bool quit = false;
    SDL_Event e;
    size_t currentImage = 0;
    //Uint32 lastUpdate = SDL_GetTicks();  // Get the initial tim code to time through images will use later


    enum ClickSection { NONE, LEFT, RIGHT };
    ClickSection clickSection = NONE;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // 1. Listen for window resize events
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                // Re-render the image
                SDL_RenderClear(renderer);

                // Calculate the destination rectangle based on the aspect ratio
                int imgWidth, imgHeight;
                SDL_QueryTexture(preloadedTextures[currentImage], NULL, NULL, &imgWidth, &imgHeight);
                float aspectRatio = (float)imgWidth / imgHeight;

                int winWidth, winHeight;
                SDL_GetWindowSize(window, &winWidth, &winHeight);

                SDL_Rect destRect;
                destRect.w = std::min(imgWidth, winWidth); // Limit width to original image width
                destRect.h = winWidth / aspectRatio;

                if (destRect.h > winHeight) {
                    destRect.h = std::min(imgHeight, winHeight);  // Limit height to original image height
                    destRect.w = destRect.h * aspectRatio;
                }

                if (destRect.h > imgHeight || destRect.w > imgWidth) {
                    // If either dimension is larger than the original image dimension,
                    // limit the rectangle to the original image size.
                    destRect.w = imgWidth;
                    destRect.h = imgHeight;
                }
                destRect.x = (winWidth - destRect.w) / 2;
                destRect.y = (winHeight - destRect.h) / 2;

                // Render the image
                SDL_RenderCopy(renderer, preloadedTextures[currentImage], NULL, &destRect);
                SDL_RenderPresent(renderer);
            }
        // 2. Listen for mouse click events
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                int winWidth, winHeight;
                SDL_GetWindowSize(window, &winWidth, &winHeight);
                
                if (x < winWidth / 2) {
                    // Assuming you have declared an enum or something for LEFT and RIGHT
                    clickSection = LEFT;  
                    currentImage = (currentImage - 1 + preloadedTextures.size()) % preloadedTextures.size();
                } else {
                    clickSection = RIGHT;
                    currentImage = (currentImage + 1) % preloadedTextures.size();
                }
                // Re-render the image
                SDL_RenderClear(renderer);

                //Calculate the destination rectangle based on the aspect ratio
                int imgWidth, imgHeight;
                SDL_QueryTexture(preloadedTextures[currentImage], NULL, NULL, &imgWidth, &imgHeight);
                float aspectRatio = (float)imgWidth / imgHeight;

                SDL_Rect destRect;
                destRect.w = std::min(imgWidth, winWidth); // Limit width to original image width
                destRect.h = winWidth / aspectRatio;

                if (destRect.h > winHeight) {
                    destRect.h = std::min(imgHeight, winHeight);  // Limit height to original image height
                    destRect.w = destRect.h * aspectRatio;
                }

                if (destRect.h > imgHeight || destRect.w > imgWidth) {
                    // If either dimension is larger than the original image dimension,
                    // limit the rectangle to the original image size.
                    destRect.w = imgWidth;
                    destRect.h = imgHeight;
                }
                destRect.x = (winWidth - destRect.w) / 2;
                destRect.y = (winHeight - destRect.h) / 2;

                // Render the image
                SDL_RenderCopy(renderer, preloadedTextures[currentImage], NULL, &destRect);
                SDL_RenderPresent(renderer);
            }   
        }       
    }
    // Cleanup
    for (auto texture : preloadedTextures) {
        SDL_DestroyTexture(texture);
    }
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
