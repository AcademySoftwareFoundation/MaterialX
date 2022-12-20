#include <MaterialXGraphEditor/Graph.h>

#include <GLFW/glfw3.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

namespace
{

ed::EditorContext* g_Context = nullptr;
bool g_FirstFrame = true;

} // anonymous namespace

static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

mx::FileSearchPath getDefaultSearchPath()
{
    mx::FilePath modulePath = mx::FilePath::getModulePath();
    mx::FilePath installRootPath = modulePath.getParentPath();
    mx::FilePath devRootPath = installRootPath.getParentPath().getParentPath();

    mx::FileSearchPath searchPath;
    if ((devRootPath / "libraries").exists())
    {
        searchPath.append(devRootPath);
    }
    else
    {
        searchPath.append(installRootPath);
    }

    return searchPath;
}

const std::string options =
" Options: \n"
"    --path [FILEPATH]              Specify an additional absolute search path location (e.g. '/projects/MaterialX').  This path will be queried when locating standard data libraries, XInclude references, and referenced images.\n"
"    --library [FILEPATH]           Specify an additional relative path to a custom data library folder (e.g. 'libraries/custom').  MaterialX files at the root of this folder will be included in all content documents.\n"
"    --help                         Display the complete list of command-line options\n";

template <class T> void parseToken(std::string token, std::string type, T& res)
{
    if (token.empty())
    {
        return;
    }

    mx::ValuePtr value = mx::Value::createValueFromStrings(token, type);
    if (!value)
    {
        std::cout << "Unable to parse token " << token << " as type " << type << std::endl;
        return;
    }

    res = value->asA<T>();
}

int main(int argc, char* const argv[])
{
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    std::string materialFilename = "resources//Materials//Examples//StandardSurface//";
    mx::FileSearchPath searchPath = getDefaultSearchPath();
    mx::FilePathVec libraryFolders = { "libraries" };

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;

        if (token == "--path")
        {
            searchPath.append(mx::FileSearchPath(nextToken));
        }
        else if (token == "--library")
        {
            libraryFolders.push_back(nextToken);
        }
        else if (token == "--help")
        {
            std::cout << " MaterialXGraphEditor version " << mx::getVersionString() << std::endl;
            std::cout << options << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Unrecognized command-line option: " << token << std::endl;
            std::cout << "Launch the graph editor with '--help' for a complete list of supported options." << std::endl;
            continue;
        }

        if (nextToken.empty())
        {
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        }
        else
        {
            i++;
        }
    }

    // Setup window
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
    {
        return 1;
    }

    // Determine GL and GLSL versions
#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 960, "MaterialX Graph Editor", NULL, NULL);
    if (!window)
    {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.Fonts->AddFontDefault();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool initialized = false;
    Graph* graph = new Graph(materialFilename, searchPath, libraryFolders);
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    double xpos, ypos = 0.0;
    xpos = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Node setup
        if (g_FirstFrame)
        {
            ed::Config config;
            config.SettingsFile = "BasicInteraction.json";
            g_Context = ed::CreateEditor(&config);
            const float zoomLevels[] = {
                0.1f,
                0.15f,
                0.20f,
                0.25f,
                0.33f,
                0.5f,
                0.75f,
                1.0f,
            };

            for (auto& level : zoomLevels)
                config.CustomZoomLevels.push_back(level);
        }
        ed::SetCurrentEditor(g_Context);
        if (g_FirstFrame)
            ed::NavigateToContent(0.0f);
            
        g_FirstFrame = false;

        if (!initialized)
        {
            initialized = true;
            graph->initialize();
        }
        graph->_renderer->drawContents();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        graph->drawGraph(ImVec2((float) xpos, (float) ypos));

        ImGui::Render();

        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (g_Context)
    {
        ed::DestroyEditor(g_Context);
        g_Context = nullptr;
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
