#include <stdio.h>

#include <glad/glad.h>
#include <glfw3.h>
#include <cglm/cglm.h>

#include "mapStruct.h"
#include "mapComponentStructs.h"

static const GLfloat tempVertData[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
};

void renderFrame(const GLuint program, const GLuint vertexBuffer) {
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
}

GLuint loadShaders(const char* vertexFilePath, const char* fragmentFilePath) {

    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    FILE* vertGLSL = fopen(vertexFilePath, "r");
    if (vertGLSL == NULL) {
        fprintf(stderr, "Failed to open shader %s\n", vertexFilePath);
    }

    fseek(vertGLSL, 0, SEEK_END);
    long vLength = ftell(vertGLSL);
    rewind(vertGLSL);
    char *vBuffer = malloc(vLength + 1);
    fread(vBuffer, 1, vLength, vertGLSL);

    fclose(vertGLSL);

    FILE* fragGLSL = fopen(fragmentFilePath, "r");
    if (fragGLSL == NULL) {
        fprintf(stderr, "Failed to open shader %s\n", vertexFilePath);
    }

    fseek(fragGLSL, 0, SEEK_END);
    long fLength = ftell(fragGLSL);
    rewind(fragGLSL);
    char *fBuffer = malloc(fLength + 1);
    fread(fBuffer, 1, fLength, fragGLSL);

    fclose(fragGLSL);

    GLint compile_status;
    GLint log_length;

    glShaderSource(vertexShaderID, 1, &vBuffer, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &compile_status);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &log_length);
    if (!compile_status) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
        fprintf(stderr, "%s\n", infoLog);
    }

    glShaderSource(fragmentShaderID, 1, &fBuffer, NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &compile_status);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &log_length);
    if (!compile_status) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
        fprintf(stderr, "%s\n", infoLog);
    }

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    GLint link_status;
    glGetProgramiv(programID, GL_LINK_STATUS, &link_status);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &log_length);

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    free(vBuffer);
    free(fBuffer);

    return programID;
}

int startGame(doomMap* map) {

    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW");
        return 0;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1024, 768, "Doomlooker", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to load GLAD\n");
        return 0;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertData), tempVertData, GL_STATIC_DRAW);

    GLuint programID = loadShaders( "../shaders/basicvert.glsl", "../shaders/basicfrag.glsl");

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    do{
        glClear( GL_COLOR_BUFFER_BIT );

        renderFrame(programID, vertexBuffer);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    return 1;
}
