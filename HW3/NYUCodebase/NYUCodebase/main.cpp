/*
CS-UY 3113
Geena Saji
HW 3
Due October 18, 2017
Space Invaders
Game ends when all 18 enemies are dead.
*/
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <windows.h>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;
using namespace std;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

class Vector3 {
public:
	Vector3() {};
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x;
	float y;
	float z;
};

class SheetSprite {
public:
	SheetSprite() {};
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};


class Entity {
public:
	Entity() {};
	Entity(SheetSprite sprite1, float x, float y, float z, float vel_x, float vel_y, float vel_z) : sprite(sprite1), position(x, y, z), velocity(vel_x, vel_y, vel_z) {}
	void Draw(ShaderProgram *program) {
		sprite.Draw(program);
	};
	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	SheetSprite sprite;
	float health = 0.0f; //0.0f=healthy, 1.0f=shot dead
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	//draw our arrays
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}



GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

vector<Entity*>  createEnemies(){
	vector<Entity*> enemies;   
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
	SheetSprite enemySprite = SheetSprite(spriteSheetTexture, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.5f);
	float xPosition = -3.0f;
	float yPosition = 3.0f;
	while (xPosition != -4.0f && yPosition != 0.0f){ //Push 18 enemy ships
		enemies.push_back(new Entity(enemySprite, xPosition, yPosition, 0.0, 0.5f, -0.1f, 0.0f));
		xPosition += 1.0f;
		if (xPosition == 3.0f){
			xPosition = -3.0f;
			yPosition -= 1.0f;
		}
	}
	return enemies;
}
Entity createBullets(Entity *player){
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
	SheetSprite bulletSprite = SheetSprite(spriteSheetTexture, 843.0f / 1024.0f, 977.0f / 1024.0f, 13.0f / 1024.0f, 37.0f / 1024.0f, 0.5f);
	Entity bullet1 = Entity(bulletSprite, player->position.x, player->position.y + 1.0f, 0.0f, 0.0f, 0.5f, 0.0f);
	return bullet1;
}
class GameState{
public:
	GameState(){}
	GameState(Entity *ptr){
		player = ptr;
		enemies = createEnemies();

	}
	Entity *player;
	vector<Entity*> enemies;
	vector<Entity*> bullets;
	void makeBulletVector(Entity *bullet1){
		bullets.push_back(bullet1);
	}
};

void DrawText(ShaderProgram *program, int fontTexture, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}


GameMode updateGame(SDL_Event* event, ShaderProgram* program, Matrix& modelViewMatrix, Matrix& projectionMatrix, bool done, GLuint fontText) {
	while (!done) {
		while (SDL_PollEvent(event)) {
			if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glUseProgram(program->programID);
		glClearColor(0.0f, 0.3f, 0.0f, 0.0f);   //Green background for main menu screen
		glClear(GL_COLOR_BUFFER_BIT);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		modelViewMatrix.Translate(0.0f, -3.0f, 0.0f);
		DrawText(program, fontText, "Space Invaders", 0.5f, 0.0f);

		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		modelViewMatrix.Identity();
		modelViewMatrix.Translate(-3.0f, 1.0f, 0.0f);
		DrawText(program, fontText, "Press ENTER", 0.5f, 0.0f);
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_RETURN]) {
			return STATE_GAME_LEVEL;
		}
		SDL_GL_SwapWindow(displayWindow);
	}
	return STATE_MAIN_MENU;
}
void renderEndScreen(ShaderProgram* program, Matrix& modelViewMatrix, Matrix& projectionMatrix, GLuint fontText){
	glUseProgram(program->programID);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);   //Black background for end screen
	glClear(GL_COLOR_BUFFER_BIT);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	
	modelViewMatrix.Translate(0.0f, -3.0f, 0.0f);
	DrawText(program, fontText, "You Win", 0.5f, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	modelViewMatrix.Translate(-3.0f, 1.0f, 0.0f);
	DrawText(program, fontText, "Game Over", 0.5f, 0.0f);
	SDL_GL_SwapWindow(displayWindow);

}
GameMode endGame(GameState &state){
	//check to see if all the enemies are dead using health

	float healthScore = 0.0f;
	for (size_t i = 0; i < state.enemies.size(); i++){
		if (state.enemies[i]->health == 1.0f){ //checking to see if all the enemies are dead
			healthScore++;
		}
	}
	if (healthScore >= 18.0f){ //because total 18 enemies
		return STATE_GAME_OVER;
	}
	return STATE_GAME_LEVEL;
}
//draw characters for game
void Render(ShaderProgram *program, const GameState &state, Matrix &projectionMatrix, Matrix &modelViewMatrix){
	glUseProgram(program->programID);
	//draw player
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player->position.x, state.player->position.y, state.player->position.z);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player->Draw(program);
	//draw enemies
	for (size_t i = 0; i < state.enemies.size(); i++){
		modelViewMatrix.Identity();
		modelViewMatrix.Translate(state.enemies[i]->position.x, state.enemies[i]->position.y, 0.0f);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		state.enemies[i]->Draw(program);
	}
	//draw bullets
	for (size_t i = 0; i < state.bullets.size(); i++){
		modelViewMatrix.Identity();
		modelViewMatrix.Translate(state.bullets[i]->position.x, state.bullets[i]->position.y, 0.0f);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		state.bullets[i]->Draw(program);
	}

}

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);

}
//move array of enemies left and right
void moveEnemies(GameState &state, float elapsed){
	for (size_t i = 0; i < state.enemies.size(); i++){//go to right
		state.enemies[i]->position.x += elapsed*state.enemies[i]->velocity.x;
	}
	for (size_t i = 0; i < state.enemies.size(); i++){//when enemies reach either ends, switch directions
		if (state.enemies[i]->position.x >= 4.0f || state.enemies[i]->position.x <= -4.0f){
			state.enemies[i]->velocity.x *= -1.0f;
		}
	}

}
//check if bullet collided with enemy
bool checkCollision(Entity *bullet, Entity *enemy){
	//if any are true, bullet and enemey are not intersecting
	if (
		((bullet->position.y + (bullet->sprite.size / 2)) <= (enemy->position.y - (enemy->sprite.size / 2))) || //bullet's top < enemy bottom
		((bullet->position.y - (bullet->sprite.size / 2)) >= (enemy->position.y + (enemy->sprite.size / 2))) || //bullet's bottom > enemy top
		((bullet->position.x + (bullet->sprite.size / 2)) <= (enemy->position.x - (enemy->sprite.size / 2))) || //bullet's right < enemy left 
		((bullet->position.x - (bullet->sprite.size / 2)) >= (enemy->position.x + (enemy->sprite.size / 2)))    //bullet's left > enemy right 
		){
		return false;
	}
	else{
		return true;
	}
}
//remove bullets & enemies that have collided with each other
void removeCollisions(GameState &state){
	for (size_t i = 0; i < state.bullets.size(); i++){	//for all the bullets in bullet vector
		for (size_t j = 0; j< state.enemies.size(); j++){
			if (checkCollision(state.bullets[i], state.enemies[j])){	//if bullet collides with enemy
				state.enemies[j]->health = 1.0f; 	//set enemy's health to 1.0f =dead
				state.enemies[j]->position.y = -100.0f; //move enemy from screen
				state.bullets[i]->position.x = -100.0f; //move bullet from screen
				return;
			}
		}
	}
}
//update the game state with entity movements
void updateGameState(GameState &state, float elapsed){
	moveEnemies(state, elapsed);
	//move bullets
	for (size_t i = 0; i < state.bullets.size(); i++){
		if (state.bullets[i] == nullptr){
			continue;
		}
		if (state.bullets[i]->position.y>4.0f){
			state.bullets.erase(state.bullets.begin() + i);
			i -= 1;				
		}
		else{
			state.bullets[i]->position.y += elapsed*state.bullets[i]->velocity.y;
		}
	}
	removeCollisions(state);
}
int main(int argc, char *argv[]) {
	Setup();

	SDL_Event event;
	bool done = false;
	float lastFrameTicks = 0.0f;
	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f /
		1024.0f, 0.5f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl"); //supports texture
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-5.0f, 5.0f, -4.0f, 4.0f, -1.0f, 1.0f);
	Entity playerEntity = Entity(playerSprite, 0.0f, -2.0f, 0.0f, 0.1f, 0.1f, 0.0f);
	GameState gState = GameState(&playerEntity);
	GLuint gameFont = LoadTexture("font1.png");
	GameMode mode = STATE_MAIN_MENU;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		switch (mode) {
		case STATE_MAIN_MENU:
			mode = updateGame(&event, &program, modelviewMatrix, projectionMatrix, done, gameFont);
			break;
		case STATE_GAME_LEVEL:
			
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
						done = true;
					}		
				}
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
				glClear(GL_COLOR_BUFFER_BIT);
				glUseProgram(program.programID);

				//move player left & right

				if (keys[SDL_SCANCODE_LEFT]){
					if ((playerEntity.position.x - (playerEntity.sprite.width / 2)) > -4.0f){
						playerEntity.position.x -= 2.0f *elapsed;
					}
				}
				if (keys[SDL_SCANCODE_RIGHT]){
					if ((playerEntity.position.x + (playerEntity.sprite.width / 2)) < 4.0f){
						playerEntity.position.x += 2.0f* elapsed;
					}
				}
				if (keys[SDL_SCANCODE_SPACE]){ 
					//shoot bullet
					//note bullet moves faster when space bar held down bit longer & relesed
					Entity bullet1 = createBullets(gState.player);
					gState.makeBulletVector(&bullet1);
				}

				Render(&program, gState, projectionMatrix, modelviewMatrix);
				updateGameState(gState, elapsed);
				SDL_GL_SwapWindow(displayWindow);
			
				//check if game over
				mode = endGame(gState); 
				break;
		case STATE_GAME_OVER: 
			renderEndScreen(&program, modelviewMatrix, projectionMatrix, gameFont);
			Sleep(5000);
			exit(0);
		}

	}
	
	SDL_Quit();
	return 0;
}