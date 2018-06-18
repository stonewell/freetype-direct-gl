#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "font_manager.h"
#include "text_buffer.h"
#include "render.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


ftdgl::FontManagerPtr font_manager;
ftdgl::text::TextBufferPtr buffer;
ftdgl::render::RenderPtr render;

void init()
{
    font_manager = ftdgl::CreateFontManager();
    buffer = ftdgl::text::CreateTextBuffer();
    render = ftdgl::render::CreateRender();

    ftdgl::text::color_s black  = {0.0, 0.0, 0.0, 1.0};
    ftdgl::text::color_s white  = {1.0, 1.0, 1.0, 1.0};
    ftdgl::text::color_s yellow = {1.0, 1.0, 0.0, 1.0};
    ftdgl::text::color_s grey   = {0.5, 0.5, 0.5, 1.0};
    ftdgl::text::color_s none   = {1.0, 1.0, 1.0, 0.0};

    ftdgl::FontPtr f_normal   = font_manager->CreateFontFromDesc("Monospace:size=24");
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
    ftdgl::text::markup_s japanese  = normal; japanese.font = f_japanese;
    ftdgl::text::markup_s math      = normal; math.font = f_math;

    ftdgl::text::pen_s pen = {20, 200};

    buffer->AddText(pen, normal, L"The");
    buffer->AddText(pen, normal,    L" Quick");
    buffer->AddText(pen, big,       L" brown ");
    buffer->AddText(pen, reverse,   L" fox \n");
    buffer->AddText(pen, italic,    L"jumps over ");
    buffer->AddText(pen, bold,      L"the lazy ");
    buffer->AddText(pen, normal,    L"dog.\n");
    buffer->AddText(pen, small,     L"Now is the time for all good men "
                    L"to come to the aid of the party.\n");
    buffer->AddText(pen, italic,    L"Ég get etið gler án þess að meiða mig.\n");
    buffer->AddText(pen, japanese,  L"私はガラスを食べられます。 それは私を傷つけません\n");
    buffer->AddText(pen, math,      L"ℕ ⊆ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ");
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor(0.40,0.40,0.45,1.00);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor4f(1.00,1.00,1.00,1.00);

    render->RenderText(buffer);

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    (void)window;
    glViewport(0, 0, width, height);
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

    window = glfwCreateWindow( 500, 220, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
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

    init();

    glfwShowWindow( window );
    reshape( window, 500, 220 );

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
