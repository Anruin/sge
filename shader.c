#include "shader.h"
#include <GL/glew.h>
#include <SDL_log.h>
#include <cglm/vec2.h>

#include "file.h"

static void Shader_CheckCompileErrors(const U32 Id, const pStr ShaderType) {
    I32 Success;
    I8 InfoLog[1024];

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        glGetShaderInfoLog(Id, 1024, NULL, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s Shader Compilation Error:\n%s\n\n", ShaderType, InfoLog);
    }
}

static void Shader_CheckLinkErrors(const U32 Id) {
    I32 Success;
    I8 InfoLog[1024];
    glGetProgramiv(Id, GL_LINK_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(Id, 1024, NULL, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader Program Link Error:\n%s\n\n", InfoLog);
    }
}

U32 Shader_LoadShader(const pStr ShaderPath) {
    const U32 Id = glCreateShader(GL_VERTEX_SHADER);

    U64 CodeLength;
    const pStr Code = File_Read(ShaderPath, &CodeLength);
    if (Code == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shader code");
        return InvalidId;
    }

    glShaderSource(Id, 1, &Code, NULL);
    glCompileShader(Id);

    I32 CompileStatus = GL_FALSE;
    glGetShaderiv(Id, GL_COMPILE_STATUS, &CompileStatus);

    if (CompileStatus == GL_FALSE) {
        I32 LogLength;
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &LogLength);

        if (LogLength > 0) {
            const pStr InfoLog = malloc(LogLength);
            if (InfoLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for shader info log");
                return InvalidId;
            }

            glGetShaderInfoLog(Id, LogLength, &LogLength, InfoLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", InfoLog);

            free(InfoLog);
        }

        return InvalidId;
    }

    return Id;
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
    const U32 VertexShaderId = Shader_LoadShader(VertexShaderPath);
    if (VertexShaderId == InvalidId) {
        return InvalidId;
    }
#pragma endregion

#pragma region Fragment Shader
    const U32 FragmentShaderId = Shader_LoadShader(FragmentShaderPath);
    if (FragmentShaderId == InvalidId) {
        return InvalidId;
    }
#pragma endregion

#pragma region Geometry Shader
    U32 GeometryShaderId = InvalidId;
    if (bCompilingGeometryShader) {
        GeometryShaderId = Shader_LoadShader(GeometryShaderPath);
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

    I32 LinkStatus = GL_FALSE;
    glGetProgramiv(Id, GL_LINK_STATUS, &LinkStatus);

    if (LinkStatus == GL_FALSE) {
        I32 LogLength;
        glGetProgramiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &LogLength);

        if (LogLength > 0) {
            const pStr InfoLog = malloc(LogLength);
            if (InfoLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to allocate memory for program link log");
                
                if (VertexShaderId != InvalidId) {
                    glDeleteShader(VertexShaderId);
                }

                if (FragmentShaderId != InvalidId) {
                    glDeleteShader(FragmentShaderId);
                }
                
                if (bCompilingGeometryShader && GeometryShaderId != InvalidId) {
                    glDeleteShader(GeometryShaderId);
                }

                return InvalidId;
            }

            glGetShaderInfoLog(VertexShaderId, LogLength, &LogLength, InfoLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program link log: %s", InfoLog);

            free(InfoLog);
        }

        if (VertexShaderId != InvalidId) {
            glDeleteShader(VertexShaderId);
        }

        if (FragmentShaderId != InvalidId) {
            glDeleteShader(FragmentShaderId);
        }
                
        if (bCompilingGeometryShader && GeometryShaderId != InvalidId) {
            glDeleteShader(GeometryShaderId);
        }

        return InvalidId;
    }

    glDeleteShader(VertexShaderId);
    glDeleteShader(FragmentShaderId);

    return Id;
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
