#include <windows.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include "FolderSelection.h"
#include "ImageLoader.h"
#include "ImageViewer.h"
#include "ThumbnailWindow.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "ImageLoader.h"
#include <iostream>
#include <sstream>
#include <cstdlib>





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int choice{0};
    while (choice != 1 && choice != 2){
        std::cout << "Choose: viewer=1 / tagger=2 ";
        std::cin >> choice;
    }
    PathSelectionSingleton& psInstance = PathSelectionSingleton::getInstance();

    std::cout << "Select image folder";
    std::string selectedPath = PathSelectionSingleton::getInstance().selectFolder();
    psInstance.setPath("imageFolder", selectedPath); // Store the selected path in the singleton
    std::string imageFolder = psInstance.getPath("imageFolder");
    printf("\nSelected folder: %s\n", imageFolder.c_str());




    if (choice == 2) {
        std::cout << "Select selected_tags.csv ";
        selectedPath = PathSelectionSingleton::getInstance().selectFile();
        psInstance.setPath("csv", selectedPath); // Store the selected path in the singleton
        std::string csvTags = psInstance.getPath("csv");
        printf("\nSelected csv: %s\n", csvTags.c_str());

        std::cout << "Select model";
        selectedPath = PathSelectionSingleton::getInstance().selectFile();
        psInstance.setPath("csv", selectedPath); // Store the selected path in the singleton
        std::string onnxModel = psInstance.getPath("csv");
        printf("\nSelected folder: %s\n", onnxModel.c_str());


        std::ostringstream command;
        command <<"onnx_inference_script.exe --folder " << imageFolder << " --csv " << csvTags <<  " --model " << onnxModel;  
        system(command.str().c_str());
    }

    std::cout << "Press enter to continue to image viewer...";
    std::cin.ignore();
    std::cin.get();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }



    SDL_Window* window = SDL_CreateWindow("Image Window",
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

    SDL_Window* thumbnailWindow = SDL_CreateWindow("Thumbnail Window",
                                               SDL_WINDOWPOS_CENTERED,
                                               SDL_WINDOWPOS_CENTERED,
                                               300, 150,
                                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_Renderer* thumbnailRenderer = SDL_CreateRenderer(thumbnailWindow, -1, SDL_RENDERER_ACCELERATED);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(thumbnailWindow, thumbnailRenderer);
    ImGui_ImplSDLRenderer2_Init(thumbnailRenderer);
    ImGui_ImplSDLRenderer2_CreateFontsTexture();

    std::set<std::string> imagePaths = getImagePaths(imageFolder);
    std::string currentImage;
    // Print out loaded image paths
    /*for (const auto& path : imagePaths) {
        printf("Loaded image: %s\n", path.c_str());
    }*/
    std::set<std::string> loadedTexturesPath;
    std::vector<std::string> thumbnailOrder;

    int count = 0;
    std::unordered_map<std::string, SDL_Texture*> preloadedTexturesIMG;
    std::unordered_map<std::string, SDL_Texture*> preloadedTexturesTHUMB;
    for (const auto& imagePath : imagePaths) {
        if (count >= 25) break; // Stop after 25 elements

        preloadedTexturesIMG[imagePath] = loadImage(imagePath, renderer);
        loadedTexturesPath.insert(imagePath);
        thumbnailOrder.push_back(imagePath);
        preloadedTexturesTHUMB[imagePath] = loadImage(imagePath, thumbnailRenderer);

        ++count;
    }
    std::string exePath = getExePath();
    std::string tagFile = exePath + "\\tags.json";
    std::unordered_map<std::string, std::vector<std::string>> tagMap = loadTagMap(tagFile);


    bool needsImGuiUpdate = false;  // New flag to track whether ImGui needs an update
    bool needsRenderUpdate = false;
    bool windowResized = false;
    bool initialSet=false;
    bool quit = false;
    bool needsThumbLoadUpdate = false;
    
    std::vector<std::string> filteredPaths;


    if (!imagePaths.empty()) {
        currentImage = *imagePaths.begin();
    }
                //std::cout << "main loadedTexturesPath:  " << &loadedTexturesPath << std::endl;

    ImageViewer viewer(window, renderer, preloadedTexturesIMG, currentImage, loadedTexturesPath, thumbnailOrder);
    Thumbnail thumb(100, 100, 10, thumbnailWindow, thumbnailRenderer, preloadedTexturesTHUMB, tagMap, imagePaths, loadedTexturesPath, renderer, preloadedTexturesIMG, thumbnailOrder);
    
    
    SDL_Event e;
    //Uint32 lastUpdate = SDL_GetTicks();  // Get the initial time code to time through images will use later
    const int FPS = 30; // Limiting to 30 FPS
    const int frameDelay = 1000 / FPS; // Time for each frame in milliseconds
    Uint32 frameStart;
    int frameTime;

   

    while (!quit) {
        frameStart = SDL_GetTicks(); // Get the time at the start of the loop

        ImGui_ImplSDL2_NewFrame(thumbnailWindow);  // Start the ImGui frame
        ImGui::NewFrame();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                    quit=true;
            }  else if (e.window.event == SDL_WINDOWEVENT_CLOSE){
                    quit=true;
            }  else { 
                if (e.window.windowID == SDL_GetWindowID(window)) {
                    // Events for the main window
                    viewer.handleEvents(e, needsRenderUpdate, needsImGuiUpdate, windowResized);
                } else if (e.window.windowID == SDL_GetWindowID(thumbnailWindow)) {
                    ImGui_ImplSDL2_ProcessEvent(&e);
                }
            }
        }   
            //printf("\n\nViewer Address: %p", viewer);
            // Reset the viewport to render the full-size image
            thumb.renderSearchBar(needsImGuiUpdate);
            thumb.renderThumbnailWindow(currentImage, needsRenderUpdate, needsThumbLoadUpdate);
            if (needsRenderUpdate) {
                std::cout << currentImage;
                viewer.renderImage();
                needsRenderUpdate = false;  // Reset the flag
            }
            if (needsThumbLoadUpdate) {
                thumb.renderThumbnailWindow(currentImage, needsRenderUpdate, needsThumbLoadUpdate);
                needsThumbLoadUpdate= false;
            }

            ImGui::Render();
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

            SDL_RenderPresent(renderer);
            SDL_RenderPresent(thumbnailRenderer);
            frameTime = SDL_GetTicks() - frameStart; // Calculate how long this frame took
            // Delay to achieve the desired frame rate
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }

    }
            
    // Cleanup
    for (auto [i, texture] : preloadedTexturesIMG) {
        SDL_DestroyTexture(texture);
    }
    for (auto [i, texture] : preloadedTexturesTHUMB) {
        SDL_DestroyTexture(texture);
    }
    IMG_Quit();
    SDL_DestroyTexture((SDL_Texture*)io.Fonts->TexID);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

