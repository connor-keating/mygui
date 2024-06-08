#pragma once

#include <windows.h>
#include <stdbool.h>
#include <gl/GL.h>

#include "types.h"

typedef char        GLchar;
typedef ptrdiff_t   GLsizeiptr;
typedef ptrdiff_t   GLintptr;

#pragma region OpenGL Extensions

#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1

#pragma endregion

#define GL_FUNC_SIGNATURE(ret, name, ...) typedef ret WINAPI glfunc_##name(__VA_ARGS__);

#define REQUIRED_OPENGL_FUNCTIONS                                                                                                                           \
    GL_FUNC_SIGNATURE(GLuint, glCreateShader, GLenum shaderType)                                                                                            \
    GL_FUNC_SIGNATURE(bool, wglSwapIntervalEXT, int interval)                                                                                               \
    GL_FUNC_SIGNATURE(int,  wglGetSwapIntervalEXT, void)                                                                                                \
    GL_FUNC_SIGNATURE(void, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)                                   \
    GL_FUNC_SIGNATURE(void, glUniform3fv, GLint location, GLsizei count, const GLfloat *value)                                                              \
    GL_FUNC_SIGNATURE(void, glCompileShader, GLuint shader)                                                                                                 \
    GL_FUNC_SIGNATURE(void, glShaderSource, GLuint shader, GLsizei count, const GLchar** string, const GLint *length)                                       \
    GL_FUNC_SIGNATURE(void, glGetShaderiv, GLuint shader, GLenum pname, GLint* params)                                                                      \
    GL_FUNC_SIGNATURE(void, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei* length, char *infoLog)                                           \
    GL_FUNC_SIGNATURE(GLuint, glCreateProgram, void)                                                                                                        \
    GL_FUNC_SIGNATURE(void, glAttachShader, GLuint program, GLuint shader)                                                                                  \
    GL_FUNC_SIGNATURE(void, glLinkProgram, GLuint program)                                                                                                  \
    GL_FUNC_SIGNATURE(void, glGetProgramiv, GLuint program, GLenum pname, GLint *params)                                                                    \
    GL_FUNC_SIGNATURE(void, glDeleteShader, GLuint shader)                                                                                                  \
    GL_FUNC_SIGNATURE(void, glGenVertexArrays, GLsizei n, GLuint *arrays)                                                                                   \
    GL_FUNC_SIGNATURE(void, glGenBuffers, GLsizei n, GLuint* buffers)                                                                                       \
    GL_FUNC_SIGNATURE(void, glBindVertexArray, GLuint array)                                                                                                \
    GL_FUNC_SIGNATURE(void, glBindBuffer, GLenum target, GLuint buffer)                                                                                     \
    GL_FUNC_SIGNATURE(void, glBufferData, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)                                                 \
    GL_FUNC_SIGNATURE(void, glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)      \
    GL_FUNC_SIGNATURE(void, glEnableVertexAttribArray, GLuint index)                                                                                        \
    GL_FUNC_SIGNATURE(void, glUseProgram, GLuint program)                                                                                                   \
    GL_FUNC_SIGNATURE(void, glDeleteVertexArrays, GLsizei n, const GLuint *arrays)                                                                          \
    GL_FUNC_SIGNATURE(void, glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog)                                       \
    GL_FUNC_SIGNATURE(void, glDeleteBuffers, GLsizei n, const GLuint * buffers)                                                                             \
    GL_FUNC_SIGNATURE(void, glDeleteProgram, GLuint program)                                                                                                \
    GL_FUNC_SIGNATURE(GLint, glGetUniformLocation, GLuint program, const GLchar *name)                                                                      \
    GL_FUNC_SIGNATURE(void, glGenerateMipmap, GLenum target)                                                                                                \
    GL_FUNC_SIGNATURE(void, glActiveTexture, GLenum texture)                                                                                                \
    GL_FUNC_SIGNATURE(void, glUniform1i, GLint location, GLint v0)                                                                                          \
    GL_FUNC_SIGNATURE(void, glUniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)                                                    \
    // end of list 

REQUIRED_OPENGL_FUNCTIONS

#undef GL_FUNC_SIGNATURE

#define GL_FUNC_SIGNATURE(ret, name, ...) glfunc_##name * name;

REQUIRED_OPENGL_FUNCTIONS

#undef GL_FUNC_SIGNATURE

bool opengl_load_functions(void)
{
    HMODULE module = LoadLibraryExW(L"opengl32.dll", NULL, 0 );
    if (!module) {
        OutputDebugStringW(L"opengl32.dll not found.\n");
        return false;
    }

    #define GL_FUNC_SIGNATURE(ret, name, ...)                                                       \
            name = (glfunc_##name *) wglGetProcAddress(#name);                                      \
            if (                                                                                    \
                name == 0 ||                                                                        \
                (name == (glfunc_##name *) 0x1) ||                                                             \
                (name == (glfunc_##name *) 0x2) ||                                                             \
                (name == (glfunc_##name *) 0x3) ||                                                             \
                (name == (glfunc_##name *) -1)                                                                 \
            )                                                                                       \
            {                                                                                       \
                OutputDebugStringW(L"wglGetProcAddress failed to find " #name "\n");                 \
                name = (glfunc_##name *) GetProcAddress(module, #name);                             \
            }                                                                                       \
            if (!name)                                                                              \
            {                                                                                       \
                OutputDebugStringW(L"Function gl" #name " couldn't be loaded from opengl32.dll\n");  \
                return false;                                                                       \
            }
        REQUIRED_OPENGL_FUNCTIONS
    #undef GL_FUNC_SIGNATURE

    return true;
}
