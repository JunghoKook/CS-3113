#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include <vector>

#define WINDOW_X 5.0f
#define WINDOW_Y 3.75f

class Entity {
public:

	glm::vec3 position;
	glm::vec3 movement;
	float speed;

	float xoff, yoff;
	float red;

	Entity();

	virtual void Update(float deltaTime);
	virtual void Render(ShaderProgram* program);

	bool collideWith(const Entity&) const;
	virtual void collide(const Entity&, float);
};

class Ball : public Entity {
public:
	bool leftwon = false;
	bool rightwon = false;

	void Update(float deltaTime);
	void collide(const Entity&, float);
	void restart();
};
