#include <stdio.h>
#include <assert.h>

#include <string>
#include <chrono>
#include <fstream>
#include <streambuf>

#include <GL/glew.h>
#include <GL/glfw.h>

#include "ezOptionParser.hpp"

#include "monaev.h"

static bool initGl()
{
    int major = 0, minor = 0;
    const char *verStr = (const char *)glGetString(GL_VERSION);
    if ((!verStr) || (sscanf(verStr,"%2d.%2d", &major, &minor) != 2))
    {
        major = minor = 0;
        fprintf(stderr, "Could'nt get GL version\n");
        return false;
    }
    fprintf(stderr, "OpenGL version %s\n", verStr);

    if (major < 2 || (major == 2 && minor < 1)) {
        return false;
    }

    glEnable(GL_TEXTURE_2D);

    return true;
}

static bool initGlew()
{
    glewExperimental = GL_TRUE;

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "%s Error: %s\n", __func__, glewGetErrorString(glew_status));
        return false;
    }

    fprintf(stderr, "Glew version %s\n", glewGetString(GLEW_VERSION));

    return true;
}

static void GLFWCALL keyCallback(int key, int action);
static void GLFWCALL winSizeCallback(int width, int height);
static void GLFWCALL mouseCallback(int button, int state);

static bool init(int argc, char *argv[])
{
    if (!glfwInit()) {
        printf("Failed to init glfw library\n" );
        return false;
    }

    if(!glfwOpenWindow(200, 200, 0, 0, 0, 0, 16, 0, GLFW_WINDOW)) {
        printf("Failed to create window\n" );
        glfwTerminate();
        return false;
    }

    glfwSetWindowTitle(argc > 0 ? argv[0] : "MonaEv");
    glfwSetKeyCallback(keyCallback);
    glfwSetWindowSizeCallback(winSizeCallback);
    glfwSetMouseButtonCallback(mouseCallback);

    if (!initGlew()) {
        printf("Failed to init glew library\n");
        return false;
    }
    if (!initGl()) {
        printf("Need openGl version 2.1 at least\n");
        return false;
    }

    return true;
}

static std::string stringFromFile(const std::string &filename)
{
    std::ifstream t(filename);

    if (t.is_open() == false) {
        return std::string();
    }

    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

    return str;
}

static bool getOptions(int argc, char *argv[], Options &options)
{
    // PARSE OPTS
    ez::ezOptionParser opt;
    opt.overview = "\nPolygon painter using random modifications running (mostly) on gpu.";

    opt.syntax = "\tmonaev --image path --dna path [OPTIONS]\n"
                 "\tmonaev --image path --polyvertexcount polygon_nb,vertex_nb [OPTIONS]\n"
                 "Then with the focus on the window, "
                 "Interactive Command Keys:\n\
            Esc/Enter   : quit\n\
            Space       : print dna\n\
            T           : switch displayed image/shapes\n\
            P           : pause/unpause\n\
            S           : print svg\n\
            H           : display Interactive Command Keys help\n"
                                                                 "Mouse Command:\n\tLeft double click :\tResize window to image size.";
    opt.example = "\tmonaev --image mona.png --dna mona.txt --descendantnb 100\n"
                  "\tmonaev --image bear.png --polyvertexcount 1000,4 --ssd --dropneutral\n\n";
    opt.footer = "monaev 0.0.1 "
                 "Copyright (C) 2015 rafirafi\nMIT License.\n\n"
                 "\"Normally all bugs are mine but thanks to opengl drivers this time I can't be sure.\"\n"
                 "Libs used : libtess2 ezOptionParser\n"
                 "Libs linked : libpng glew libglfw\n"
                 "Dna output is compatible with http://alteredqualia.com/visualization/evolve/\n";

    opt.add("", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Display this help.", // Help description.
            "--help"); // Flag token.

    opt.add("mona.png", 1, 1, 0, "Path to the png image to paint. [mandatory]", "--image");
    opt.add("", 0, 1, 0, "Path to a dna file associated with the image to paint.", "--dna");
    opt.add("30,3", 0, 2, ',',
            "Number of polygon (>= 1) followed by number of vertex (>= 3) to paint, separated by a comma",
            "--polyvertexcount");
    opt.add("", 0, 0, 0, "Drop neutral changes.", "--dropneutral");
    opt.add("", 0, 0, 0,
            "Evaluate with sum of squared differences instead of sum of absolute differences.",
            "--ssd");
    opt.add("25", 0, 1, 0, "Nb of descendant per generation.", "--descendantnb");

    opt.add("1", 0, 1, 0,
            "Divide polygon rendering for reuse [1,polygon count], "
            "if 0 calibrate for performance, if not set use 1 slice. "
            "Evaluation results slightly change with the nb of slice.",
            "--slicenb");

    opt.add("", 0, 0, 0,
            "Always uses 16 bit textures for evaluation calculations. Always set for opengl 2.1. "
            "Variable decrease of the performance, increase of the gpu load.",
            "--force16bit");

    opt.parse(argc, (const char **)argv);

    auto Usage = [&](){
        std::string usage;
        opt.getUsage(usage);
        std::cout << usage;
    };

    if (opt.isSet("--help")) {
        Usage();
        return false;
    }

    std::vector<std::string> badOptions;
    int i;
    if(!opt.gotRequired(badOptions)) {
        for(i=0; i < (int)badOptions.size(); ++i) {
            printf("ERROR: Missing required option %s .\n", badOptions[i].c_str());
        }
        Usage();
        return false;
    }
    if(!opt.gotExpected(badOptions)) {
        for(i=0; i < (int)badOptions.size(); ++i) {
            printf("ERROR: Got unexpected number of arguments for option %s .\n", badOptions[i].c_str());
        }
        Usage();
        return false;
    }

    std::string dnaPath;
    opt.get("--image")->getString(options.painter.imagePath);
    opt.get("--dna")->getString(dnaPath);

    if (dnaPath.empty()) {
        std::vector<int> polyVertCnt;
        opt.get("--polyvertexcount")->getInts(polyVertCnt);
        if (polyVertCnt.size() != 2 || polyVertCnt[0] < 1 || polyVertCnt[1] < 3) {
            printf("ERROR: Got unexpected arguments for option --polyvertexcount .\n");
            Usage();
            return false;
        }
        options.manager.polygonCount = polyVertCnt[0];
        options.manager.vertexCount = polyVertCnt[1];
    } else {
        options.manager.dna = stringFromFile(dnaPath);
    }

    opt.get("--descendantnb")->getInt(options.painter.stepSize);
    options.painter.stepSize = std::max(1, options.painter.stepSize);
    opt.get("--slicenb")->getInt(options.manager.sliceCount);
    options.evaluator.useSSd = opt.isSet("--ssd");
    options.painter.dropNeutral = opt.isSet("--dropneutral");
    options.evaluator.force16Bit = opt.isSet("--force16bit");

    return true;
}

static bool g_isRunning;
static bool g_isPaused;
static bool g_doDumpDna;
static bool g_doDumpSvg;
static bool g_doChangeTexture;
static bool g_doResize = true;
static std::array<int, 2> g_winSize;

static void GLFWCALL winSizeCallback(int width, int height)
{
    g_winSize = {{ width, height }};
}

static void GLFWCALL mouseCallback(int button, int state)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || state != GLFW_PRESS) {
        return;
    }
    static std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    auto now = std::chrono::high_resolution_clock::now();
    int msElapsed = static_cast<int>(std::chrono::duration_cast
                                     <std::chrono::milliseconds>
                                     (now - lastTime).count());
    lastTime = now;
    if (msElapsed < 500) {
        g_doResize = true;
    }
}

static void GLFWCALL keyCallback(int key, int action)
{
    if (action == GLFW_PRESS) {
        if ((key == GLFW_KEY_ESC
             || key == GLFW_KEY_ENTER)) { // quit
            g_isRunning = false;
        } else if (key == GLFW_KEY_SPACE) { // space . dna
            g_doDumpDna = true;
            g_isPaused = true;
        } else if (key == 'T') { // texture
            g_doChangeTexture = true;
        } else if (key == 'P') { // pause
            g_isPaused = !g_isPaused;
        } else if (key == 'S') { // svg
            g_doDumpSvg = true;
            g_isPaused = true;
        } else if (key == 'H') {
            g_isPaused = true;
            printf("\
    Interactive Command Keys:\n\
    Esc/Enter   : quit\n\
    Space       : print dna\n\
    T           : switch displayed image/shapes\n\
    P           : pause/unpause\n\
    S           : print svg\n\
    H           : display this help\n");
        }
    }
}

int main(int argc, char *argv[])
{
    Options options;
    if (getOptions(argc, argv, options) == false) {
        return -1;
    }

    if (init(argc, argv) == false) {
        return -1;
    }

    MonaEv *mona = new MonaEv();
    if (mona->init(options) == false) {
        delete mona;
        return -1;
    }

    bool showImage = false;
    g_isRunning = true;
    while (g_isRunning)
    {
        if (g_doResize) {
            g_doResize = false;
            g_winSize = mona->getimageSize();
            glfwSetWindowSize(g_winSize[0], g_winSize[1]);
        }

        if (g_doDumpDna == true) {
            mona->printDna();
            g_doDumpDna = false;
        }

        if (g_doDumpSvg == true) {
            mona->printSvg();
            g_doDumpSvg = false;
        }

        if (g_doChangeTexture == true) {
            showImage = !showImage;
            g_doChangeTexture = false;
        }

        if (g_isPaused) {
            if (showImage) {
                mona->displayImage(g_winSize);
            } else {
                mona->displayShapes(g_winSize);
            }
            glfwSleep(0.05);

            glfwSwapBuffers();
        } else {
            mona->printInfo();
            mona->doStep();

            if (glfwGetTime() > 1. / 10.) { // fps
                glfwSetTime(0.);

                if (showImage) {
                    mona->displayImage(g_winSize);
                } else {
                    mona->displayShapes(g_winSize);
                }
                assert(glGetError() == GL_NO_ERROR);

                glfwSwapBuffers();
            }
        }

        glfwPollEvents();

        if (!glfwGetWindowParam(GLFW_OPENED) ) {
            g_isRunning = false;
        }
    }

    delete mona;

    glfwCloseWindow();
    glfwTerminate();

    return 0;
}
