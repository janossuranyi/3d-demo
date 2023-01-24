#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


void demo_1();

struct Test {
    Test() = default;
    ~Test() {
        Info("Test::dtor");
    }
    int x;
};

void* xmalloc(size_t size) {
    Info("xmalloc %d", size);
    return malloc(size);
}

int main(int argc, char** argv)
{


    JseInit(argc, argv);


    try {
        JseGfxRenderer R;
        R.InitCore(1024, 768, false, true);
        for (int i = 0; i < 20; ++i) {
            R.Frame();
        }
    }
    catch (std::exception e) { Error("error=%s", e.what()); }

    JseShutdown();

	Info("Program terminated");

	return 0;
}