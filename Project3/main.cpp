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

struct GameState {
	Entity player;
	Entity* objects;
	int objectUsed;
	int objectCount;
	Entity* platform;
	bool ended = false;
	bool success = false;
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
		vertices.insert(vertices.end(), {   offset + (-0.5f * size), (-0.5f * size),
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
	displayWindow = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 1280, 720);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	fontTextureID = LoadTexture("font.png");

	state.player.textureID = LoadTexture("player.png");
	state.player.position = glm::vec3(-6, 2.5, 0);
	state.player.acceleration = glm::vec3(0, -0.1f, 0);
	
	state.player.entityType = PLAYER;


	state.objects = new Entity[state.objectCount = 80];
	state.objectUsed = 0;
	
	// left
	for (int i = 0; i < 9; i++) {
		state.objects[state.objectUsed].position = glm::vec3(-8.0f + 0.5f, 4.5f - 0.5f - i * 1.0f, 0);
		state.objectUsed++;
	}
	// right
	for (int i = 0; i < 9; i++) {
		state.objects[state.objectUsed].position = glm::vec3(8.0f - 0.5f, 4.5f - 0.5f - i * 1.0f, 0);
		state.objectUsed++;
	}
	// top
	for (int i = 0; i < 14; i++) {
		state.objects[state.objectUsed].position = glm::vec3(-8.0f + 1.5f + i * 1.0f, 4.5f - 0.5f, 0);
		state.objectUsed++;
	}
	// bottom
	for (int i = 0; i < 14; i++) {
		state.objects[state.objectUsed].position = glm::vec3(-8.0f + 1.5f + i * 1.0f, -4.5f + 0.5f, 0);
		state.objectUsed++;
	}

	// obstacle
	state.objects[state.objectUsed].position = glm::vec3(4.5f, -3.0f, 0);
	state.objectUsed++;

	GLuint rocktex = LoadTexture("rock.png");
	for (int i = 0; i < state.objectUsed; i++) {
		state.objects[i].textureID = rocktex;
		state.objects[i].entityType = ROCK;
	}
	int pindex = 44;
	state.objects[pindex].textureID  = state.objects[pindex + 1].textureID = LoadTexture("platform.png");
	state.objects[pindex].entityType = state.objects[pindex + 1].entityType = PLATFORM;
	state.platform = &state.objects[pindex];

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
				// Some sort of action
				break;

			}
			break;
		}
	}

	// Check for pressed/held keys below
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LEFT])
	{
		state.player.acceleration.x = -0.5;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		state.player.acceleration.x = 0.5;
	}
	else
	{
		state.player.acceleration.x = 0.0;
	}

	if (keys[SDL_SCANCODE_UP])
	{
		state.player.acceleration.y = 0.2;
	}
	else
	{
		state.player.acceleration.y = -0.2;
	}


	if (keys[SDL_SCANCODE_R] && state.ended)
	{
		state.player.position = glm::vec3(-6, 2.5, 0);
		state.player.velocity = glm::vec3(0);
		state.player.acceleration = glm::vec3(0, -0.1f, 0);
		state.player.lastCollision = NONE;

		state.ended = false;
		state.success = false;
	}
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
	if (state.player.lastCollision != NONE) {
		state.ended = true;
		state.success = false;
		if (state.player.lastCollision == PLATFORM) {
			state.success = true;
		}
		return;
	}

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
		state.player.Update(FIXED_TIMESTEP, state.objects, state.objectUsed);

		deltaTime -= FIXED_TIMESTEP;
	}

	accumulator = deltaTime;

}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	state.player.Render(&program);

	for (int i = 0; i < state.objectUsed; i++) {
		state.objects[i].Render(&program);
	}
	
	if (state.ended) {
		if (state.success) {
			DrawText(&program, fontTextureID, "Mission Successful", 0.5f, -0.2f, glm::vec3(-2, 2, 0));
		}
		else {
			DrawText(&program, fontTextureID, "Mission Failed", 0.5f, -0.2f, glm::vec3(-2, 2, 0));
		}
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
