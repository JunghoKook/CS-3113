#pragma once
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

enum EntityType { NONE, PLAYER, PLATFORM, ROCK };

class Entity {
public:

	EntityType entityType;
	EntityType lastCollision;

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	float width, height;

	GLuint textureID;

	Entity();

	bool CheckCollision(Entity);
	void CheckCollisionsY(Entity*, int);
	void CheckCollisionsX(Entity*, int);
	void Update(float, Entity*, int);
	void Render(ShaderProgram*);
};


