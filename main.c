#include <am.h>
#include <display.h>
#include <klib-macros.h>
#include <screen.h>

int main(const char *args) {
    const char *fmt = "Hello, AbstractMachine!\n"
                      "mainargs = '%'.\n";

    for (const char *p = fmt; *p; p++) {
        (*p == '%') ? putstr(args) : putch(*p);
    }
    ioe_init();
    setScreenResolution(128, 128);
    changePalette(1, 0xffd5);
    for (int v = 0; v < 128; v++) {
        setPix(v, 1, 1);
        setPix(v, 2, 1);
        setPix(v, 3, 1);
    }
    redrawScreen();
    while (1) {
        ;
        ;
    }
    return 0;
}
