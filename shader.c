#include "shader.h"
#include <GL/glew.h>
#include <SDL_log.h>
#include <cglm/vec2.h>

#include "file.h"

#define SHADER_LOG_LENGTH 1024

static pStr Shader_GetShaderTypeString(const EShaderType ShaderType) {
    static const pStr ShaderTypeUnknown = "Unknown";
    static const pStr ShaderTypeVertex = "Vertex";
    static const pStr ShaderTypeFragment = "Fragment";
    static const pStr ShaderTypeGeometry = "Geometry";

    switch (ShaderType) {
    case Shader_Vertex:
        return ShaderTypeVertex;
    case Shader_Fragment:
        return ShaderTypeFragment;
    case Shader_Geometry:
        return ShaderTypeGeometry;
    default:
        return ShaderTypeUnknown;
    }
}

static Bool Shader_CheckCompileSucceeded(const U32 Id, const EShaderType ShaderType) {
    I32 CompileStatus = False;
    I8 InfoLog[SHADER_LOG_LENGTH];

    glGetShaderiv(Id, GL_COMPILE_STATUS, &CompileStatus);

    if (!CompileStatus) {
        glGetShaderInfoLog(Id, SHADER_LOG_LENGTH, NULL, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s Shader Compilation Error:\n%s\n\n", Shader_GetShaderTypeString(ShaderType), InfoLog);
    }

    return CompileStatus;
}

static Bool Shader_CheckLinkSucceeded(const U32 Id) {
    I32 LinkStatus;
    I8 InfoLog[SHADER_LOG_LENGTH];

    glGetProgramiv(Id, GL_LINK_STATUS, &LinkStatus);

    if (!LinkStatus) {
        glGetProgramInfoLog(Id, SHADER_LOG_LENGTH, NULL, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader Program Link Error:\n%s\n\n", InfoLog);
    }

    return LinkStatus;
}

U32 Shader_LoadShader(const pStr ShaderPath, const EShaderType ShaderType) {
    U32 Id;

    switch (ShaderType) {
    case Shader_Vertex:
        Id = glCreateShader(GL_VERTEX_SHADER);
        break;
    case Shader_Fragment:
        Id = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case Shader_Geometry:
        Id = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    default:
        return InvalidId;
    }

    I64 CodeLength;
    const pStr Code = File_ReadText(ShaderPath, &CodeLength);
    if (Code == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shader code");
        return InvalidId;
    }

    glShaderSource(Id, 1, &Code, NULL);
    glCompileShader(Id);

    I32 CompileStatus = GL_FALSE;
    glGetShaderiv(Id, GL_COMPILE_STATUS, &CompileStatus);

    if (Shader_CheckCompileSucceeded(Id, ShaderType)) {
        return Id;
    }

    glDeleteShader(Id);

    return InvalidId;
}

U32 Shader_LoadProgram(const pStr VertexShaderPath, const pStr FragmentShaderPath, const pStr GeometryShaderPath) {
    if (VertexShaderPath == NULL || SDL_strcmp(VertexShaderPath, StrEmpty) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vertex shader path is empty");
        return InvalidId;
    }

    if (FragmentShaderPath == NULL || SDL_strcmp(FragmentShaderPath, StrEmpty) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fragment shader path is empty");
        return InvalidId;
    }

    Bool bCompilingGeometryShader = False;
    if (GeometryShaderPath != NULL && SDL_strcmp(GeometryShaderPath, StrEmpty) != 0) {
        bCompilingGeometryShader = True;
    }

#pragma region Vertex Shader
    const U32 VertexShaderId = Shader_LoadShader(VertexShaderPath, Shader_Vertex);
    if (VertexShaderId == InvalidId) {
        return InvalidId;
    }
#pragma endregion

#pragma region Fragment Shader
    const U32 FragmentShaderId = Shader_LoadShader(FragmentShaderPath, Shader_Fragment);
    if (FragmentShaderId == InvalidId) {
        return InvalidId;
    }
#pragma endregion

#pragma region Geometry Shader
    U32 GeometryShaderId = InvalidId;
    if (bCompilingGeometryShader) {
        GeometryShaderId = Shader_LoadShader(GeometryShaderPath, Shader_Geometry);
        if (GeometryShaderId == InvalidId) {
            return InvalidId;
        }
    }
#pragma endregion

    const U32 Id = glCreateProgram();

    glAttachShader(Id, VertexShaderId);
    glAttachShader(Id, FragmentShaderId);
    if (bCompilingGeometryShader) {
        glAttachShader(Id, GeometryShaderId);
    }

    glLinkProgram(Id);

    if (VertexShaderId != InvalidId) {
        glDeleteShader(VertexShaderId);
    }

    if (FragmentShaderId != InvalidId) {
        glDeleteShader(FragmentShaderId);
    }

    if (bCompilingGeometryShader && GeometryShaderId != InvalidId) {
        glDeleteShader(GeometryShaderId);
    }

    if (Shader_CheckLinkSucceeded(Id)) {
        return Id;
    }

    glDeleteProgram(Id);

    return InvalidId;
}

void Shader_Use(const U32 Id) {
    glUseProgram(Id);
}

void Shader_SetBool(const U32 Id, const pStr Name, const Bool Value) {
    glUniform1i(glGetUniformLocation(Id, Name), Value);
}

void Shader_SetI32(const U32 Id, const pStr Name, const I32 Value) {
    glUniform1i(glGetUniformLocation(Id, Name), Value);
}

void Shader_SetF32(const U32 Id, const pStr Name, const F32 Value) {
    glUniform1f(glGetUniformLocation(Id, Name), Value);
}

void Shader_SetVector2V(const U32 Id, const pStr Name, const vec2 Value) {
    glUniform2fv(glGetUniformLocation(Id, Name), 1, &Value[0]);
}

void Shader_SetVector2(const U32 Id, const pStr Name, const F32 X, const F32 Y) {
    glUniform2f(glGetUniformLocation(Id, Name), X, Y);
}

void Shader_SetVector3V(const U32 Id, const pStr Name, const vec3 Value) {
    glUniform3fv(glGetUniformLocation(Id, Name), 1, &Value[0]);
}

void Shader_SetVector3(const U32 Id, const pStr Name, const F32 X, const F32 Y, const F32 Z) {
    glUniform3f(glGetUniformLocation(Id, Name), X, Y, Z);
}

void Shader_SetVector4V(const U32 Id, const pStr Name, const vec4 Value) {
    glUniform4fv(glGetUniformLocation(Id, Name), 1, &Value[0]);
}

void Shader_SetVector4(const U32 Id, const pStr Name, const F32 X, const F32 Y, const F32 Z, const F32 W) {
    glUniform4f(glGetUniformLocation(Id, Name), X, Y, Z, W);
}

void Shader_SetMatrix2(const U32 Id, const pStr Name, const mat2 Matrix) {
    glUniformMatrix2fv(glGetUniformLocation(Id, Name), 1, GL_FALSE, &Matrix[0][0]);
}

void Shader_SetMatrix3(const U32 Id, const pStr Name, const mat3 Matrix) {
    glUniformMatrix3fv(glGetUniformLocation(Id, Name), 1, GL_FALSE, &Matrix[0][0]);
}

void Shader_SetMatrix4(const U32 Id, const pStr Name, const mat4 Matrix) {
    glUniformMatrix4fv(glGetUniformLocation(Id, Name), 1, GL_FALSE, &Matrix[0][0]);
}
