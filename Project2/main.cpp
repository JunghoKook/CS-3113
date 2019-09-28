#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Entity.h"

#include <vector>

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

float lastTicks = 0;

Entity p1;
Entity p2;
Ball ball;
std::vector<Entity*> ents;

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

	ents.push_back(&p1);
	ents.push_back(&p2);
	ents.push_back(&ball);

	p1.position = glm::vec3(-5.0f, 0.0f, 0.0f);
	p2.position = glm::vec3(5.0f, 0.0f, 0.0f);
	p1.xoff = p2.xoff = 0.1f;
	p1.yoff = p2.yoff = 0.8f;
	p1.speed = p2.speed = 4.0f;

	ball.restart();
	ball.xoff = 0.1f;
	ball.yoff = 0.1f;
	ball.speed = 5.0f;

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-WINDOW_X, WINDOW_X, -WINDOW_Y, WINDOW_Y, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(program.programID);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void ProcessInput() {
	p1.movement = glm::vec3(0, 0, 0);
	p2.movement = glm::vec3(0, 0, 0);

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

	// p1
	if (keys[SDL_SCANCODE_A])
	{
		p1.movement.x = -1;
	}
	else if (keys[SDL_SCANCODE_D])
	{
		p1.movement.x = 1;
	}

	if (keys[SDL_SCANCODE_W])
	{
		p1.movement.y = 1;
	}
	else if (keys[SDL_SCANCODE_S])
	{
		p1.movement.y = -1;
	}

	//p2
	if (keys[SDL_SCANCODE_LEFT])
	{
		p2.movement.x = -1;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		p2.movement.x = 1;
	}

	if (keys[SDL_SCANCODE_UP])
	{
		p2.movement.y = 1;
	}
	else if (keys[SDL_SCANCODE_DOWN])
	{
		p2.movement.y = -1;
	}


	//if (glm::length(p1.movement) > 1.0f)
	//{
	//	p1.movement = glm::normalize(p1.movement);
	//}
	//if (glm::length(p2.movement) > 1.0f)
	//{
	//	p2.movement = glm::normalize(p2.movement);
	//}
	for (Entity* e : ents) {
		if (glm::length(e->movement) > 1.0f)
		{
			e->movement = glm::normalize(e->movement);
		}
	}
}

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	for (Entity* e : ents) {
		e->Update(deltaTime);
		for (Entity* other : ents) {
			e->collide(*other, deltaTime);
		}
	}

	if (ball.leftwon) {
		p1.red = 0.0f;
		p2.red = 1.0f;
		ball.leftwon = false;
	}
	else if (ball.rightwon) {
		p1.red = 1.0f;
		p2.red = 0.0f;
		ball.rightwon = false;
	}
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	for (Entity* e : ents) {
		e->Render(&program);
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
