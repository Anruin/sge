#pragma once
#include <GL/glew.h>
#include <cglm/cglm.h>
#include "typedefs.h"

typedef struct {
    GLuint ProgramId;
    GLuint TextureId;
    GLuint Texture;

    mat4 Projection;
    mat4 View;
    GLuint TransformMatrixId;
    GLuint ModelMatrixId;

    vec3 CameraPosition;
    vec3 CameraForward;
    vec3 CameraRight;
    vec3 CameraUp;
    F32 CameraDistance;
    F32 CameraClippingDistance;
    GLuint CameraId;
    GLuint DefaultVertexArrayId;
} RenderService;

RenderService* RenderService_Get();
void RenderService_Initialize();
void RenderService_Shutdown();
void RenderService_Update(RenderService* pRenderService, F32 DeltaTime);
GLuint RenderService_LoadShaders(RenderService* pRenderService, pStr VertexShaderPath, pStr FragmentShaderPath);
GLuint RenderService_LoadDDS(RenderService* pRenderService, pcStr TexturePath);
