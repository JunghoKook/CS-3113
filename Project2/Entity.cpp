#include "Entity.h"

Entity::Entity()
{
	position = glm::vec3(0);
	speed = 0;
	xoff = 0;
	yoff = 0;
	red = 1.0f;
}

void Entity::Update(float deltaTime)
{
	position += movement * speed * deltaTime;
	if (position.x - xoff < -WINDOW_X) {
		position.x = -WINDOW_X + xoff;
	}
	else if (position.x + xoff > WINDOW_X) {
		position.x = WINDOW_X - xoff;
	}

	if (position.y - yoff < -WINDOW_Y) {
		position.y = -WINDOW_Y + yoff;
	}
	else if (position.y + yoff > WINDOW_Y) {
		position.y = WINDOW_Y - yoff;
	}
}

void Entity::Render(ShaderProgram* program) {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);

	float vertices[] = { -xoff, -yoff, xoff, -yoff, xoff, yoff,
		-xoff, -yoff, xoff, yoff, -xoff, yoff };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	program->SetColor(red, 1.0f, 1.0f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
}

bool Entity::collideWith(const Entity& e) const {
	return xoff + e.xoff > glm::abs<float>(position.x - e.position.x)
		&& yoff + e.yoff > glm::abs<float>(position.y - e.position.y);
}

void Entity::collide(const Entity& e, float deltaTime) {
	if (&e == this) {
		return;
	}
	if (collideWith(e)) {
		position -= movement * speed * deltaTime;
	}
}

void Ball::collide(const Entity& e, float deltaTime) {
	if (&e == this) {
		return;
	}
	if (collideWith(e)) {
		movement.x = -movement.x;
	}
}

void Ball::Update(float deltaTime)
{
	position += movement * speed * deltaTime;
	if (position.x - xoff < -WINDOW_X) {
		restart();
		leftwon = false;
		rightwon = true;
	}
	else if (position.x + xoff > WINDOW_X) {
		restart();
		leftwon = true;
		rightwon = false;
	}

	if (position.y - yoff < -WINDOW_Y) {
		position.y = -WINDOW_Y + yoff;
		movement.y = -movement.y;
	}
	else if (position.y + yoff > WINDOW_Y) {
		position.y = WINDOW_Y - yoff;
		movement.y = -movement.y;
	}
}

void Ball::restart() {
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	movement = glm::vec3(1, 1, 0);
}
