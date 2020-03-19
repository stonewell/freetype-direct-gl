#define GL_SILENCE_DEPRECATION
#include "opengl.h"
#ifndef __APPLE__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "font_manager.h"
#include "text_buffer.h"
#include "render.h"
#include "screenshot-util.h"

#ifdef GLFW_HACK_FIX
#include "glfw_hack.h"
#endif

ftdgl::FontManagerPtr font_manager;
ftdgl::text::TextBufferPtr buffer;
ftdgl::render::RenderPtr render;

void init(const ftdgl::viewport::viewport_s & viewport)
{
    if (!font_manager)
        font_manager = ftdgl::CreateFontManager(viewport.dpi, viewport.dpi_height);

    buffer = ftdgl::text::CreateTextBuffer(viewport);

    if (!render)
        render = ftdgl::render::CreateRender();

    ftdgl::text::color_s black  = {0.0, 0.0, 0.0, 1.0};
    ftdgl::text::color_s white  = {1.0, 1.0, 1.0, 1.0};
    ftdgl::text::color_s yellow = {1.0, 1.0, 0.0, 1.0};
    ftdgl::text::color_s grey   = {0.5, 0.5, 0.5, 1.0};
    ftdgl::text::color_s none   = {1.0, 1.0, 1.0, 0.0};

    ftdgl::FontPtr f_normal   = font_manager->CreateFontFromDesc("Monospace:size=16");
    ftdgl::FontPtr f_small   = font_manager->CreateFontFromDesc("Monospace:size=10");
    ftdgl::FontPtr f_big   = font_manager->CreateFontFromDesc("Monospace:size=48:slant=italic");
    ftdgl::FontPtr f_bold     = font_manager->CreateFontFromDesc("Droid Serif:size=24:weight=200");
    ftdgl::FontPtr f_italic   = font_manager->CreateFontFromDesc("Droid Serif:size=24:slant=italic");
    ftdgl::FontPtr f_japanese = font_manager->CreateFontFromDesc("Droid Sans:size=18:lang=ja");
    ftdgl::FontPtr f_math     = font_manager->CreateFontFromDesc("DejaVu Sans:size=24");

    ftdgl::text::markup_s normal = {
        .fore_color    = white, .back_color    = none,
        .font = f_normal,
    };
    ftdgl::text::markup_s highlight = normal; highlight.back_color = grey;
    ftdgl::text::markup_s reverse   = normal; reverse.fore_color = black;
                                 reverse.back_color = white;
    ftdgl::text::markup_s small     = normal; small.font = f_small;
    ftdgl::text::markup_s big       = normal; big.font = f_big;
                                 big.fore_color = yellow;
    ftdgl::text::markup_s bold      = normal; bold.font = f_bold;
    ftdgl::text::markup_s italic    = normal; bold.font = f_italic;
    ftdgl::text::markup_s japanese  = normal; japanese.font = f_japanese; japanese.back_color = grey;
    ftdgl::text::markup_s math      = normal; math.font = f_math;math.fore_color = yellow;
    math.back_color = grey;

    ftdgl::text::pen_s pen = {40, 250};

    (void)none;
    buffer->AddText(pen, normal, L"The");
    buffer->AddText(pen, normal,    L" Quick");
    buffer->AddText(pen, big,       L" brown ");
    buffer->AddText(pen, reverse,   L" fox \n"); pen.x = 20;
    buffer->AddText(pen, italic,    L"jumps over ");
    buffer->AddText(pen, bold,      L"the lazy ");
    buffer->AddText(pen, normal,    L"dog.\n"); pen.x = 20;
    buffer->AddText(pen, small,     L"Now is the time for all good men "
                    L"to come to the aid of the party.\n"); pen.x = 20;
    buffer->AddText(pen, italic,    L"Ég get etið gler án þess að meiða mig.\n"); pen.x = 20;
    buffer->AddText(pen, japanese,  L"私はガラスを食べられます。 それは私を傷つけません\n"); pen.x = 20;
    buffer->AddText(pen, math,      L"ℕ ⊆ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂけま哈哈\n"); pen.x = 20;
    buffer->AddText(pen, big, L"pork\n"); pen.x = 20;
    buffer->AddText(pen, normal, L"Arial");
}

void setup(GLFWwindow* window) {
    int pixel_height = 0, pixel_width = 0;
    glfwGetFramebufferSize(window, &pixel_width, &pixel_height);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int widthMM, heightMM;
    glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &widthMM, &heightMM);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float dpi = mode->width / (widthMM / 25.4) * (float)pixel_width / (float)width;
    float dpi_height = (mode->height / (heightMM / 25.4)) * (float)pixel_height / (float)height;

    std::cout << "pw:" << pixel_width << ", ph:" << pixel_height
              << ", w:" << width << ", h:" << height
              << ", dpi:" << dpi
              << ", " << dpi_height
              << ", mw:" << mode->width << ", mh:" << mode->height
              << std::endl;

    ftdgl::viewport::viewport_s viewport {
        pixel_width, pixel_height,
        dpi, dpi_height,
        0, 0
    };

    init(viewport);
    init(viewport);
}

// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.40,0.40,0.45,1.00);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if (buffer && render) {
        render->RenderText(buffer);
        render->RenderText(buffer);
    }

#ifdef GLFW_HACK_FIX
    updateGlfwContext(window);
#endif

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    (void)window;
    (void)width;
    (void)height;
    glViewport(0, 0, width, height);

    printf("w:%d, h:%d\n", width, height);
    setup(window);
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    (void)scancode;
    (void)mods;
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    (void)error;
    fputs( description, stderr );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    GLFWwindow* window;
    char* screenshot_path = NULL;

    if (argc > 1)
    {
        if (argc == 3 && 0 == strcmp( "--screenshot", argv[1] ))
            screenshot_path = argv[2];
        else
        {
            fprintf( stderr, "Unknown or incomplete parameters given\n" );
            exit( EXIT_FAILURE );
        }
    }

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow( 800, 440, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        fprintf(stderr, "error create window\n");
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

#ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

    reshape(window, 800 * 2, 440 * 2);

    glfwShowWindow( window );

    while(!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
