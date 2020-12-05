#pragma once
#include <cglm/types.h>

#include "typedefs.h"

typedef enum {
    Shader_Unknown = -1,
    Shader_Vertex,
    Shader_Fragment,
    Shader_Geometry
} EShaderType;

/** Loads and compiles shaders. */
U32 Shader_LoadProgram(pStr VertexShaderPath, pStr FragmentShaderPath, pStr GeometryShaderPath);
/** Use the shader. */
void Shader_Use(U32 Id);
/** Set the shader uniform value. */
void Shader_SetBool(U32 Id, pStr Name, Bool Value);
/** Set the shader uniform value. */
void Shader_SetI32(U32 Id, pStr Name, I32 Value);
/** Set the shader uniform value. */
void Shader_SetF32(U32 Id, pStr Name, F32 Value);
/** Set the shader uniform value. */
void Shader_SetVector2V(U32 Id, pStr Name, const vec2 Value);
/** Set the shader uniform value. */
void Shader_SetVector2(U32 Id, pStr Name, F32 X, F32 Y);
/** Set the shader uniform value. */
void Shader_SetVector3V(U32 Id, pStr Name, const vec3 Value);
/** Set the shader uniform value. */
void Shader_SetVector3(U32 Id, pStr Name, F32 X, F32 Y, F32 Z);
/** Set the shader uniform value. */
void Shader_SetVector4V(U32 Id, pStr Name, const vec4 Value);
/** Set the shader uniform value. */
void Shader_SetVector4(U32 Id, pStr Name, F32 X, F32 Y, F32 Z, F32 W);
/** Set the shader uniform value. */
void Shader_SetMatrix2(U32 Id, pStr Name, const mat2 Matrix);
/** Set the shader uniform value. */
void Shader_SetMatrix3(U32 Id, pStr Name, const mat3 Matrix);
/** Set the shader uniform value. */
void Shader_SetMatrix4(U32 Id, pStr Name, const mat4 Matrix);
