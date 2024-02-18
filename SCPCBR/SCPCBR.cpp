#include <iostream>
#include <Windows.h>

#include "GameAssembly/GameAssembly.h"
#include "Localization/Localization.h"
#include "SteamWrapper/SteamWrapper.h"
#include "AudioEngine/AudioEngine.h"
#include "Options/Options.h"
#include "Launcher/Launcher.h"
#include "SplashScreen/SplashScreen.h"
#include "PreloadManager/PreloadManager.h"
#include "MainMenu/MainMenu.h"

#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <GLAD/glad.h>

#include <string>

GLFWwindow* window;

GlobalGameState CurrentGlobalGameState;

int main(int argc, char* argv[])
{
    CurrentGlobalGameState = GlobalGameState::Launcher;
    
    // Hide console window
    ShowWindowAsync(GetConsoleWindow(), SW_HIDE);

    SteamWrapper* steam = nullptr;
    
    // Initialize GLFW
    if (!glfwInit()) {
        exit(EXIT_FAILURE);  // NOLINT(concurrency-mt-unsafe)
    }

    // Set OpenGL & GLFW info
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create GLFW window
    window = glfwCreateWindow(640, 410, GameWindowTitle, 0, 0);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Set window context
    glfwMakeContextCurrent(window);

    // Disable vSync
    glfwSwapInterval(0);

    // Set window icon
    Window::SetWindowIcon(window, GameWindowIcon);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = "uicfg.ini";

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    AudioEngine::Init();
    Localization::Init();
    
    Launcher::Init();
    SplashScreen::Init();
    PreloadManager::Init();

    AudioEngine::CreateChannelGroup("Game");
    AudioEngine::CreateChannelGroup("Music");

    AudioEngine::SetChannelGroupVolume("Game", Options::ReadIntOption("Audio", "GameVolume") / 100.0f);
    AudioEngine::SetChannelGroupVolume("Music", Options::ReadIntOption("Audio", "MusicVolume") / 100.0f);
    
    while (!glfwWindowShouldClose(window)) {
        if (steam) {
            steam->RunCallbacks();
        }
        
        AudioEngine::RunCallbacks();
        
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

#ifdef _DEBUG
        ImGui::GetForegroundDrawList()->AddText(ImVec2(1, 1), ImColor(255, 255, 255), std::string(std::to_string((int)ImGui::GetIO().Framerate) + " FPS").c_str());
#endif

        switch (CurrentGlobalGameState) {
            case GlobalGameState::Launcher: {
                Launcher::Render(window, &CurrentGlobalGameState);
                break;
            }
            case GlobalGameState::Splash: {
                SplashScreen::Render(window, &CurrentGlobalGameState, steam);
                break;
            }
            case GlobalGameState::Preload: {
                PreloadManager::Render(window, &CurrentGlobalGameState);
                break;
            }
            case GlobalGameState::MainMenu: {
                break;
            }
            case GlobalGameState::Game: {
                break;
            }
        }

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }

    PreloadManager::Free();
    SplashScreen::Free();
    Launcher::Free();
    
    Localization::Free();
    AudioEngine::Free();

    glfwTerminate();

    delete steam;
    
    exit(EXIT_SUCCESS);  // NOLINT(concurrency-mt-unsafe)
}