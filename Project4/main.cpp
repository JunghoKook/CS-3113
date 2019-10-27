#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

#define PLATFORM_COUNT 15
#define ENEMY_COUNT 3

struct GameState {
    Entity player;
    Entity platforms[PLATFORM_COUNT];
	Entity enemies[ENEMY_COUNT];
	bool gamewon = false;
};

GameState state;

GLuint fontTextureID;

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

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position) {
	float width = 1.0f / 16.0f;
	float height = 1.0f / 16.0f;

	std::vector<float> vertices;
	std::vector<float> texCoords;

	for (int i = 0; i < text.size(); i++) {
		int index = (int)text[i];

		float u = (float)(index % 16) / 16.0f;
		float v = (float)(index / 16) / 16.0f;

		texCoords.insert(texCoords.end(), { u, v + height, u + width, v + height, u + width, v,
											u, v + height, u + width, v, u, v });

		float offset = (size + spacing) * i;
		vertices.insert(vertices.end(), { offset + (-0.5f * size), (-0.5f * size),
											offset + (0.5f * size), (-0.5f * size),
											offset + (0.5f * size), (0.5f * size),
											offset + (-0.5f * size), (-0.5f * size),
											offset + (0.5f * size), (0.5f * size),
											offset + (-0.5f * size), (0.5f * size) });

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		program->SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, fontTextureID);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2.0f);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	fontTextureID = LoadTexture("font.png");
    
    state.player.entityType = PLAYER;
    state.player.isStatic = false;
    state.player.width = 1.0f;
    state.player.position = glm::vec3(-4, 3, 0);
    state.player.acceleration = glm::vec3(0, -9.81f, 0);
    state.player.textureID = LoadTexture("me.png");
    
    GLuint dirtTextureID = LoadTexture("dirt.png");
    GLuint grassTextureID = LoadTexture("grass.png");
    
    for (int i = 0; i < 11; i++)
    {
        state.platforms[i].textureID = dirtTextureID;
        state.platforms[i].position = glm::vec3(i - 5.0f, -3.25f, 0);
    }
    
    state.platforms[11].textureID = grassTextureID;
    state.platforms[11].position = glm::vec3(-5.0f, -2.25f, 0);
    
    state.platforms[12].textureID = grassTextureID;
    state.platforms[12].position = glm::vec3(-4.0f, -2.25f, 0);

    state.platforms[13].textureID = grassTextureID;
    state.platforms[13].position = glm::vec3(4.5f, -2.25f, 0);

	state.platforms[14].textureID = grassTextureID;
	state.platforms[14].position = glm::vec3(2.5f, -2.25f, 0);

	GLuint enemyTextureID = LoadTexture("evil.png");

	for (int i = 0; i < ENEMY_COUNT; i++) {
		state.enemies[i].isStatic = false;
		state.enemies[i].textureID = enemyTextureID;
	}

	state.enemies[0].entityType = ENEMY_JUMP;
	state.enemies[0].position = state.platforms[13].position;
	state.enemies[0].position.y += 2.0f;
	state.enemies[0].acceleration = glm::vec3(0, -9.81f, 0);

	state.enemies[1].entityType = ENEMY_PATROL;
	state.enemies[1].velocity.x = 3.0f;
	state.enemies[1].acceleration = glm::vec3(0, -9.81f, 0);

	state.enemies[2].entityType = ENEMY_TELEPORT;

	

    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				state.player.Jump();
				break;

			}
			break;
		}
	}

	state.player.velocity.x = 0;

	// Check for pressed/held keys below
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_A])
	{
		state.player.velocity.x = -3.0f;
	}
	else if (keys[SDL_SCANCODE_D])
	{
		state.player.velocity.x = 3.0f;
	}
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	deltaTime += accumulator;
	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}

	while (deltaTime >= FIXED_TIMESTEP) {
		// Update. Notice it's FIXED_TIMESTEP. Not deltaTime
		if (!state.player.gameover) {
			state.player.Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);

			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				state.enemies[i].Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, &state.player, 1);
			}
		}
		deltaTime -= FIXED_TIMESTEP;
	}

	int deadEnemies = 0;
	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (!state.enemies[i].isActive) {
			deadEnemies += 1;
		}
	}
	if (deadEnemies == ENEMY_COUNT) {
		state.gamewon = true;
	}

	accumulator = deltaTime;
}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	state.player.Render(&program);

	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		state.enemies[i].Render(&program);
	}

	for (int i = 0; i < PLATFORM_COUNT; i++)
	{
		state.platforms[i].Render(&program);
	}

	if (state.player.gameover) {
		DrawText(&program, fontTextureID, "Game Over", 0.5f, -0.2f, glm::vec3(-1.0, 2, 0));
	}
	else if (state.gamewon) {
		DrawText(&program, fontTextureID, "You Win", 0.5f, -0.2f, glm::vec3(-1.0, 2, 0));
	}

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