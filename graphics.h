#pragma once
#include <SDL_video.h>
#include <cglm/cglm.h>
#include "typedefs.h"

typedef struct {
    SDL_GLContext pSDL_GlContext;
    
    U32 ProgramId;
    U32 TextureId;
    U32 Texture;

    mat4 Projection;
    mat4 View;
    U32 TransformMatrixId;
    U32 ModelMatrixId;

    vec3 CameraPosition;
    vec3 CameraForward;
    vec3 CameraRight;
    vec3 CameraUp;
    F32 CameraDistance;
    F32 CameraClippingDistance;
    U32 CameraId;
    U32 DefaultVertexArrayId;
} FRenderService;

/** Returns the render service singleton. */
FRenderService* RenderService_Get();

/** Initializes the render service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void RenderService_Initialize(FRenderService* pRenderService);

/** Updates the render service frame. */
void RenderService_Tick(FRenderService* pRenderService, F32 DeltaTime);

/** Clears the initialized buffers, shaders, textures and frees memory. */
void RenderService_Shutdown();
