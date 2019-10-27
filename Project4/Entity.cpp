#include "Entity.h"

Entity::Entity()
{
    entityType = PLATFORM;
    isStatic = true;
    isActive = true;
	gameover = false;
    position = glm::vec3(0);
    time = 0;
    width = 1;
    height = 1;
}

bool Entity::CheckCollision(Entity other)
{
    if (isStatic == true) return false;
    if (isActive == false || other.isActive == false) return false;
    
    float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
    float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);

    if (xdist < 0 && ydist < 0)
    {
        if (entityType == PLAYER && other.entityType == COIN)
        {
            other.isActive = false;
        }
        
        return true;
    }
    
    return false;
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object.position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object.position.x);
            float penetrationX = fabs(xdist - (width / 2) - (object.width / 2));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}


void Entity::Jump()
{
    if (collidedBottom)
    {
        velocity.y = 5.0f;
    }
}




void Entity::Update(float deltaTime, Entity *objects, int objectCount, Entity *enemies, int enemyCount)
{
	if (isStatic) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    
    velocity += acceleration * deltaTime;
    
    position.y += velocity.y * deltaTime;        // Move on Y
    CheckCollisionsY(objects, objectCount);    // Fix if needed
    
    position.x += velocity.x * deltaTime;        // Move on X
    CheckCollisionsX(objects, objectCount);    // Fix if needed

	switch (entityType) {
	case ENEMY_JUMP:
		Jump();
		break;
	case ENEMY_PATROL:
		if (collidedRight) {
			velocity.x = -3.0f;
		}
		else if (collidedLeft) {
			velocity.x = 3.0f;
		}
		break;
	case ENEMY_TELEPORT:
		if (enemies && enemyCount == 1) {
			if (time >= 0) { // not active
				time += deltaTime;
				if (time > 0.8f) {
					time = -3.0f;
					position = lastPosition;
				}
			}
			else { // active
				time += deltaTime;
				if (time > 0) {
					time = 0;
					position.x = -100.0f;
					lastPosition = enemies->position;
				}
			}
		}
		else {
			isActive = false;
		}
		break;
	case PLAYER:
		if (enemies) {
			for (int i = 0; i < enemyCount; i++) {
				if (CheckCollision(enemies[i])) {
					if (position.y + 0.1f - height / 2 > enemies[i].position.y) {
						enemies[i].isActive = false;
					}
					else {
						gameover = true;
					}
				}
			}
		}
		break;
	}
}



void Entity::Render(ShaderProgram *program) {
	if (isActive == false) return;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

