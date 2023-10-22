#include "ImageViewer.h"
#include <algorithm> // for std::min
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <iostream>
// Enum to represent the section of the window that was clicked.

// Constructor: Initialize properties
ImageViewer::ImageViewer(SDL_Window* window, SDL_Renderer* initrenderer, std::unordered_map<std::string, SDL_Texture*>& textures, std::string& initcurrentImage, std::set<std::string>& initloadedTexturesPath, std::vector<std::string>& initthumbnailOrder)
    : window(window), renderer(initrenderer), preloadedTextures(textures), currentImage(initcurrentImage), loadedTexturesPath(initloadedTexturesPath), thumbnailOrder(initthumbnailOrder) {
            std::cout << "imageviewer loadedTexturesPath:  " << &loadedTexturesPath << std::endl;
    }

// Function to handle SDL Events
void ImageViewer::handleEvents(SDL_Event& e, bool& needsRenderUpdate, bool& needsImGuiUpdate, bool& windowResized) {
    // Handle window resize events
    if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
        // Re-render the imag
                windowResized = true;
                renderImage(); // Call the render function
    } 
    // Handle mouse click events
    else if (e.type == SDL_MOUSEBUTTONDOWN) {
        // ... (Same logic as you have for mouse click)
                int x, y;
                SDL_GetMouseState(&x, &y);
                int winW, winH;
                SDL_GetWindowSize(window, &winW, &winH);
                            //std::cout << "imageviewer loadedTexturesPath mouseclick:  " << &loadedTexturesPath << std::endl;

                if (x < winW / 2) {
                    // Scroll to the previous image
                    auto it = std::find(thumbnailOrder.begin(), thumbnailOrder.end(), currentImage);
                    if (it != thumbnailOrder.end()) {
                        if (it != thumbnailOrder.begin()) {
                            --it;  // Move to the previous image if not at the beginning
                            currentImage = *it;
                        } else {
                            // Loop back to the last image if at the beginning
                            currentImage = *thumbnailOrder.rbegin();
                        }
                    }
                } else {
                        // Scroll to the next image
                        auto it = std::find(thumbnailOrder.begin(), thumbnailOrder.end(), currentImage);
                            if (it != thumbnailOrder.end()) {
                                ++it;  // Move to the next image
                                if (it != thumbnailOrder.end()) {
                                    currentImage = *it;
                                } else {
                                // Loop back to the first image if at the end
                                currentImage = *thumbnailOrder.begin();
                            }
                        }
                    }
                renderImage();
                needsRenderUpdate = true;  // Flag to indicate that the image needs to be re-rendered
            }
        }

// Function to render the current image
void ImageViewer::renderImage() {
    //std::cout << "imageviewer loadedTexturesPath Lazy Load:  " << &loadedTexturesPath << std::endl;
    const char* error = SDL_GetError();
        if (*error != '\0') {
        printf("SDL Error before segfault: %s\n", error);
        SDL_ClearError();
    }
    if (renderer == nullptr) {
    printf("Renderer is NULL.\n");
    
    } else {
    SDL_RenderSetViewport(renderer, NULL);
    }
    //SDL_RenderSetViewport(renderer, NULL);
    // Clear the renderer
    SDL_RenderClear(renderer);

    // Calculate the destination rectangle for the image
    SDL_Rect destRect = calculateDestRect();

    // Render the image
    // Get the original index of the current image
    
    SDL_Texture* currentTexture = preloadedTextures[currentImage];
    SDL_RenderCopy(renderer, currentTexture, NULL, &destRect);
    SDL_RenderPresent(renderer);
}

// Function to calculate and return the destination rectangle for rendering the image
SDL_Rect ImageViewer::calculateDestRect() {
    // ... (Same logic as you have for calculating destRect)
    // ...
    int imgWidth, imgHeight;
    SDL_QueryTexture(preloadedTextures[currentImage], NULL, NULL, &imgWidth, &imgHeight);
    float aspectRatio = (float)imgWidth / imgHeight;

    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    
    SDL_Rect destRect;
    destRect.w = std::min(imgWidth, winWidth); // Limit width to original image width
    destRect.h = winWidth / aspectRatio;
    //printf("\nwinWidth: %i, winHeight: %i\n", winWidth, winHeight);
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
    //printf("\ndestRect.x: %i, destRect.y: %i\n", destRect.x, destRect.y);
    return destRect;
}