#include "font_manager.h"

int main() {
    ftdgl::FontManagerPtr fm = ftdgl::CreateFontManager(72, 72);

    fm->CreateFontFromDesc("Serif-12:lang=en:weight=200");
    fm->CreateFontFromDesc("Serif-12:lang=ko:weight=200");
    fm->CreateFontFromDesc("Serif-12:lang=ja:weight=200");
    fm->CreateFontFromDesc("Serif-12:lang=zh-CN:weight=200");
    fm->CreateFontFromDesc("Serif-12:lang=en:weight=80");
    fm->CreateFontFromDesc("Serif-12:lang=ko:weight=80");
    fm->CreateFontFromDesc("Serif-12:lang=ja:weight=80");
    fm->CreateFontFromDesc("Serif-12:lang=zh-CN:weight=80");
    fm->CreateFontFromDesc("Monospace-12:lang=en:weight=200");
    fm->CreateFontFromDesc("Monospace-12:lang=ko:weight=200");
    fm->CreateFontFromDesc("Monospace-12:lang=ja:weight=200");
    fm->CreateFontFromDesc("Monospace-12:lang=zh-CN:weight=200");
    fm->CreateFontFromDesc("Monospace-12:lang=en:weight=80");
    fm->CreateFontFromDesc("Monospace-12:lang=ko:weight=80");
    fm->CreateFontFromDesc("Monospace-12:lang=ja:weight=80");
    fm->CreateFontFromDesc("Monospace-12:lang=zh-CN:weight=80");
    return 0;
}
