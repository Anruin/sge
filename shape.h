#pragma once
#include <GL/glew.h>

#include "typedefs.h"
#include "containers/vector.h"

#define SIZE_VERTEX (sizeof(F32))
#define SIZE_COLOR (sizeof(F32))
#define SIZE_TEXCOORD (sizeof(F32))
#define SIZE_NORMAL (sizeof(F32))
#define SIZE_INDEX (sizeof(U16))

typedef struct FShape {
    FVector(F32) Vertices;
    FVector(F32) Colors;
    FVector(F32) TexCoords;
    FVector(F32) Normals;
    FVector(U16) Indices;
} FShape;

void Shape_Buffer(FShape Shape, U32* IndexBuffer, U32* VertexBuffer, U32* ColorBuffer, U32* TexCoordBuffer, U32* NormalBuffer, U32 ShaderProgram);
