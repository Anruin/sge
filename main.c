#include "application.h"

int main(int argc, char* argv[]) {
    FApplication* App = Application_Get();

    Application_Initialize(App);
    Application_Run(App);
    Application_Shutdown(App);

    return 0;
}
