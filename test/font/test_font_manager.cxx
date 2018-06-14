#include "font_manager.h"

int main() {
    ftdgl::FontManagerPtr fm = ftdgl::CreateFontManager();

    fm->CreateFontFromDesc("Serif-12:lang=en:weight=40");
    return 0;
}
