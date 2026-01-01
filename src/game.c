#include <stdio.h>

#include <glad/glad.h>
#include <glfw3.h>
#include <cglm/cglm.h>

#include "buildModel.h"

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1500

#define MOUSE_SENS 0.05f
#define MOVE_SPEED 100.0f

void renderFrame(const GLuint program, const GLuint vertexBuffer, const size_t vertCount, vec3 camPos, vec3 direction) {
    glUseProgram(program);

    mat4 model, view, proj, pv, mvp;
    glm_perspective(glm_rad(45.0f), (float) WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 4000.0f, proj);

    vec3 center;
    glm_vec3_add(camPos, direction, center);
    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_lookat(camPos, center, up, view);
    glm_mat4_identity(model);
    vec3 translation = {0.0f, 0.0f, 0.0f};
    glm_translate(model, translation);

    glm_mat4_mul(proj, view, pv);
    glm_mat4_mul(pv, model, mvp);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    const GLint matID = glGetUniformLocation(program, "mvp");
    glUniformMatrix4fv(matID, 1, GL_FALSE, &mvp[0][0]);


    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) vertCount);
    glDisableVertexAttribArray(0);
}

GLuint loadShaders(const char* vertexFilePath, const char* fragmentFilePath) {

    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    FILE* vertGLSL = fopen(vertexFilePath, "rb");
    if (vertGLSL == NULL) {
        fprintf(stderr, "Failed to open shader %s\n", vertexFilePath);
    }

    fseek(vertGLSL, 0, SEEK_END);
    long vLength = ftell(vertGLSL);
    rewind(vertGLSL);
    char *vBuffer = malloc(vLength + 1);
    fread(vBuffer, 1, vLength, vertGLSL);
    vBuffer[vLength] = '\0';


    fclose(vertGLSL);

    FILE* fragGLSL = fopen(fragmentFilePath, "rb");
    if (fragGLSL == NULL) {
        fprintf(stderr, "Failed to open shader %s\n", vertexFilePath);
    }

    fseek(fragGLSL, 0, SEEK_END);
    long fLength = ftell(fragGLSL);
    rewind(fragGLSL);
    char *fBuffer = malloc(fLength + 1);
    fread(fBuffer, 1, fLength, fragGLSL);
    fBuffer[fLength] = '\0';

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

void handleInputs(GLFWwindow* window, vec3 camPos, vec2 lookAngle, vec3 direction, double deltaTime) {
    glfwPollEvents();

    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);

    int centreX = WINDOW_WIDTH/2;
    int centreY = WINDOW_HEIGHT/2;

    int relCursorX = (int)cursorX - centreX;
    int relCursorY = (int)cursorY - centreY;

    glfwSetCursorPos(window, centreX, centreY);

    lookAngle[0] -= MOUSE_SENS * (float) deltaTime * (float) relCursorX;
    lookAngle[1] -= MOUSE_SENS * (float) deltaTime * (float) relCursorY;

    if (lookAngle[1] <= -1.55){lookAngle[1] =  -1.55f;}
    if (lookAngle[1] >= 1.55){lookAngle[1] =  1.55f;}

    direction[0] = cosf(lookAngle[1]) * sinf(lookAngle[0]);
    direction[1] = sinf(lookAngle[1]);
    direction[2] = cosf(lookAngle[1]) * cosf(lookAngle[0]);

    vec3 right = {sinf(lookAngle[0] - CGLM_PI / 2.0f), 0.0f, cosf(lookAngle[0] - CGLM_PI / 2.0f)};
    vec3 forward;
    glm_normalize_to(direction, forward);

    vec3 cross, mov_x, mov_y, mov_z;
    glm_cross(right, direction, cross);

    float speedFactor = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speedFactor *= 2;
    }

    glm_vec3_scale(right, (float) deltaTime * MOVE_SPEED * speedFactor, mov_x);
    glm_vec3_scale(cross, (float) deltaTime * MOVE_SPEED * speedFactor, mov_y);
    glm_vec3_scale(forward, (float) deltaTime * MOVE_SPEED * speedFactor, mov_z);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 output_position = {};
        glm_vec3_add(camPos, mov_z, output_position);
        glm_vec3_copy(output_position, camPos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 output_position = {};
        glm_vec3_sub(camPos, mov_z, output_position);
        glm_vec3_copy(output_position, camPos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 output_position = {};
        glm_vec3_sub(camPos, mov_x, output_position);
        glm_vec3_copy(output_position, camPos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 output_position = {};
        glm_vec3_add(camPos, mov_x, output_position);
        glm_vec3_copy(output_position, camPos);
    }

}

int startGame(const mapModel* map) {

    float* vertCoords = map->vertCoords;
    size_t vertCount = map->vertCount;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW");
        return 0;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Doomlooker", NULL, NULL);

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
    glBufferData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(float), vertCoords, GL_STATIC_DRAW);

    GLuint programID = loadShaders( "../shaders/basicvert.glsl", "../shaders/basicfrag.glsl");

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    vec3 camPos = {0.0f, 0.0f, 5.0f};
    vec2 lookAngle = {3.14f, 0.0f};
    vec3 direction = {};

    double currentTime = glfwGetTime();
    do{
        glClear( GL_COLOR_BUFFER_BIT );

        double newTime = glfwGetTime();
        double deltaTime = newTime - currentTime;
        currentTime = glfwGetTime();

        handleInputs(window, camPos, lookAngle, direction, deltaTime);

        renderFrame(programID, vertexBuffer, vertCount, camPos, direction);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    return 1;
}
