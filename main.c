#include <SDL.h>

#include "application.h"

FApplication App;

int main(int argc, char* argv[]) {
    Application_Initialize(&App);
    Application_Run(&App);
    Application_Shutdown(&App);

    return 0;
}
