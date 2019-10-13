#include "Entity.h"

Entity::Entity()
{
	entityType = lastCollision = NONE;

	position = glm::vec3(0);
	velocity = glm::vec3(0);
	acceleration = glm::vec3(0);

	width = height = 1.0f;
}

bool Entity::CheckCollision(Entity other) {
	float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
	float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);

	if (xdist < 0 && ydist < 0) {
		lastCollision = other.entityType;
		return true;
	}
	return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount) {
	for (int i = 0; i < objectCount; i++) {
		Entity object = objects[i];

		if (CheckCollision(object)) {
			float ydist = fabs(position.y - object.position.y);
			float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
			if (velocity.y > 0) {
				position.y -= penetrationY;
				velocity.y = 0;
			}
			else if (velocity.y < 0) {
				position.y += penetrationY;
				velocity.y = 0;
			}
		}
	}
}
void Entity::CheckCollisionsX(Entity* objects, int objectCount) {
	for (int i = 0; i < objectCount; i++) {
		Entity object = objects[i];

		if (CheckCollision(object)) {
			float xdist = fabs(position.x - object.position.x);
			float penetrationX = fabs(xdist - (width / 2) - (object.width/ 2));
			if (velocity.x > 0) {
				position.x -= penetrationX;
				velocity.x = 0;
			}
			else if (velocity.x < 0) {
				position.x += penetrationX;
				velocity.x = 0;
			}
		}
	}
}

void Entity::Update(float deltaTime, Entity* objects, int objectCount) 
{
	if (entityType == PLAYER) {
		velocity += acceleration * deltaTime;

		position.y += velocity.y * deltaTime;
		CheckCollisionsY(objects, objectCount);
		position.x += velocity.x * deltaTime;
		CheckCollisionsX(objects, objectCount);
	}
	else if (entityType == PLATFORM) {
		return;
	}
	else if (entityType == ROCK) {
		return;
	}
}

void Entity::Render(ShaderProgram* program) {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);

	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
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
