#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
ShaderProgram program_ut;

glm::mat4 viewMatrix, projectionMatrix;
glm::mat4 p1Matrix, p2Matrix, triMatrix;

float player_x = 0;

GLuint p1TextureID;
GLuint p2TextureID;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Textured", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	program_ut.Load("shaders/vertex.glsl", "shaders/fragment.glsl");
    
	p1TextureID = LoadTexture("p1.png");
	p2TextureID = LoadTexture("p2.png");
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
	program_ut.SetProjectionMatrix(projectionMatrix);
	program_ut.SetViewMatrix(viewMatrix);
	program_ut.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

float lastTicks = 0;
float rotate_z = 0;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    player_x += 1.0f * deltaTime;
    
    rotate_z += 45.0 * deltaTime;

    p1Matrix = glm::mat4(1.0f);
	p1Matrix = glm::translate(p1Matrix, glm::vec3(player_x, 0.0f, 0.0f));
	p1Matrix = glm::rotate(p1Matrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	p2Matrix = glm::mat4(1.0f);
	p2Matrix = glm::translate(p2Matrix, glm::vec3(-player_x, 0.0f, 0.0f));
	p2Matrix = glm::rotate(p2Matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	triMatrix = glm::mat4(1.0f);
	triMatrix = glm::rotate(triMatrix, glm::radians(rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program.programID);
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

	program.SetModelMatrix(p1Matrix);
	glBindTexture(GL_TEXTURE_2D, p1TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(p2Matrix);
	glBindTexture(GL_TEXTURE_2D, p2TextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    

	glUseProgram(program_ut.programID);

	float vertices_ut[] = { 0.5, -0.5, 0.0, 0.5, -0.5, -0.5 };

	glVertexAttribPointer(program_ut.positionAttribute, 2, GL_FLOAT, false, 0, vertices_ut);
	glEnableVertexAttribArray(program_ut.positionAttribute);

	program_ut.SetModelMatrix(triMatrix);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(program_ut.positionAttribute);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
