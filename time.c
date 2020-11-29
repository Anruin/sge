#include "time.h"

#include <SDL_timer.h>

#define FRAME_NUMBER 10

#pragma region Private Variables
/** An array to store frame times. */
static U32 FrameTimes[FRAME_NUMBER];
/** Last frame time. */
static U32 LastFrameTime;
/** Total frame count. */
static U32 FrameCount;
/** Delta time in microseconds from the last frame. */
static U32 DeltaTime;
/** Average frames per second value for the last FrameNumber frames. */
static F32 FramesPerSecond;
/** Total ticks count. */
static U32 Ticks;
#pragma endregion

#pragma region Private Function Declarations
static void TimeService_CalculateFps();
#pragma endregion

#pragma region Public Function Definitions
void TimeService_Initialize() {
    LastFrameTime = SDL_GetTicks();
}

U32 TimeService_Tick() {
    Ticks = SDL_GetTicks();
    DeltaTime = Ticks - LastFrameTime;

    TimeService_CalculateFps();

    return DeltaTime;
}

void TimeService_Shutdown() {
}
#pragma endregion

#pragma region Private Function Definitions
void TimeService_CalculateFps() {
    const U32 FrameTimesIndex = FrameCount % FRAME_NUMBER;
    FrameTimes[FrameTimesIndex] = DeltaTime;
    LastFrameTime = Ticks;
    FrameCount++;

    U32 Count;

    if (FrameCount < FRAME_NUMBER) {
        Count = FrameCount;
    } else {
        Count = FRAME_NUMBER;
    }

    FramesPerSecond = 0;
    for (U32 Index = 0; Index < Count; Index++) {
        FramesPerSecond += FrameTimes[Index];
    }

    FramesPerSecond /= Count;

    FramesPerSecond = 1000.f / FramesPerSecond;
}
#pragma endregion
