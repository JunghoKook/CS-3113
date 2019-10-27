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

enum  EntityType { PLAYER, PLATFORM, COIN, ENEMY_JUMP, ENEMY_PATROL, ENEMY_TELEPORT };


class Entity {
public:
    
    EntityType entityType;
    bool isStatic;
    bool isActive;
	bool gameover;
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
	glm::vec3 lastPosition;
    
    float width;
    float height;
    
    float time;
    
    GLuint textureID;
    
    Entity();
    
    bool CheckCollision(Entity other);
    
    void CheckCollisionsX(Entity *objects, int objectCount);
    void CheckCollisionsY(Entity *objects, int objectCount);
    
    void Update(float deltaTime, Entity *objects, int objectCount, Entity* = nullptr, int = 0);
    void Render(ShaderProgram *program);
    
    void Jump();
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    
};



