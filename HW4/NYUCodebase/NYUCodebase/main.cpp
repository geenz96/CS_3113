/*
CS-UY 3113
Geena Saji
HW 4
Due November 8, 2017
Simple Platformer Game
Ideally Game ends if player hits enemy.
Display screen does not appear correctly every time code is run.
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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define FIXED_TIMESTEP 0.01f
#define TILE_SIZE 0.2f
#define LEVEL_HEIGHT 32
#define LEVEL_WIDTH 128
#define SPRITE_COUNT_X 4
#define SPRITE_COUNT_Y 2

SDL_Window* displayWindow;
using namespace std;

enum EntityType { ENTITY_PLAYER, ENTITY_ENEMY };
int** levelData;
class Entity;
vector<Entity*> entityVect;
int mapWidth; 
int mapHeight;

//Divide unit coordinates by the tile size and invert Y
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}
//linear interpolation
float lerp(float v0, float v1, float t){
	return (1.0 - t)*v0 + t*v1;
}

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
SheetSprite playerSprite;
SheetSprite enemySprite;

class Entity {
public:
	Entity() {};
	Entity(SheetSprite sprite1, EntityType entType, bool static_status, float sz_x, float sz_y, float x, float y, float z, 
		float vel_x, float vel_y, float vel_z, float accel_x, float accel_y, float fric_x, float fric_y) 
		: sprite(sprite1), entityType(entType), isStatic(static_status), size(sz_x, sz_y,0.0f), position(x, y, z), 
		velocity(vel_x, vel_y, vel_z), acceleration(accel_x,accel_y,0.0f), friction(fric_x,fric_y,0.0f) {}
	void Draw(ShaderProgram &program) {
		sprite.Draw(&program);
	};
	
	void Update(float elapsed){
		velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
		velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);		
		velocity.x += acceleration.x * elapsed;
		velocity.y += acceleration.y * elapsed;
		position.x += velocity.x * elapsed;
		position.y += velocity.y * elapsed;
	};

	void CollidesWith(Entity *entity){
		//left collision: player left > entity right
		if ((position.x - (sprite.size / 2)) >= (entity->position.x + (entity->sprite.size / 2))){
			position.x += 1.0f;
			entity->position.x -= 1.0f;
			collidedLeft = true;
		}
		//right collision: player right <entity left
		if ((position.x + (sprite.size / 2)) <= (entity->position.x - (entity->sprite.size / 2))){
			position.x -= 1.0f;
			entity->position.x += 1.0f;
			collidedRight = true;
		}
		//bottom collision: player bottom > entity top 
		if ((position.y - (sprite.size / 2)) >= (entity->position.y + (entity->sprite.size / 2))){
			position.y += 1.0f;
			entity->position.y -= 1.0f;
			collidedBottom = true;
		}
		//top collision: player top < entity bottom
		if ((position.y + (sprite.size / 2)) <= (entity->position.y - (entity->sprite.size / 2))){
			position.y -= 1.0f;
			entity->position.y += 1.0f;
			collidedTop = true;
		}
	}; 

	SheetSprite sprite;
	Vector3 position;
	Vector3 size;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 friction;

	bool isStatic;
	EntityType entityType;
	
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

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

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);

}
void createSprite(){
	GLuint gameSpriteSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	playerSprite = SheetSprite(gameSpriteSheet, 0.0f / 128.0f, 64.0f / 128.0f, 16.0f / 256.0f, 32.0f / 128.0f, 1.0f);
}
bool readHeader(ifstream &stream) { //reading the header
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new int*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new int[mapWidth];
		}
		return true;
	}
}

bool readLayerData(ifstream &stream) { //reading the tile data
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					int val = atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}
void placeEntity(string entType, float x_pos, float y_pos){
	createSprite();
	if (entType == "Player"){
		entityVect.push_back(new Entity(playerSprite, ENTITY_PLAYER, false, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f));
	}
	else if (entType == "Enemy"){
		entityVect.push_back(new Entity(playerSprite, ENTITY_ENEMY, false, 1.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f));
	}	
}
void drawCharacters(ShaderProgram &program, Matrix &modelviewMatrix, Matrix &projectionMatrix){
	for (int i = 0; i < entityVect.size(); i++){
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(entityVect[i]->position.x, entityVect[i]->position.y, 0);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetModelviewMatrix(modelviewMatrix);
		entityVect[i]->Draw(program);
	}
}
bool readEntityData(ifstream &stream) { //reading the entity data
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}
//for each tile in map draw to screen
void renderTileMap(ShaderProgram* program, GLuint spriteTexture){
	vector<float> vertexData;
	vector<float> texCoordData;
	int counter = 0;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x] !=0){
				counter++;
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});
				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				});
			}
			
		}
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	glDrawArrays(GL_TRIANGLES, 0, counter*6); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
} 
//move screen as player moves
void centerOnPlayer(ShaderProgram* program, Matrix& modelViewMatrix){
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(entityVect[0]->position.x, entityVect[0]->position.y, entityVect[0]->position.z);
	program->SetModelviewMatrix(modelViewMatrix);
}

int main(int argc, char *argv[]) {
	Setup();
	SDL_Event event;
	bool done = false;
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	GLuint spriteSheetTexture = LoadTexture("arne_sprites.png");
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl"); 
	projectionMatrix.SetOrthoProjection(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
	float lastFrameTicks = 0.0f;
	float accum = 0.0f;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	//read file
	ifstream infile("gamemap.txt");
	string line;
	while(getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				assert(false);
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Player]") {
			readEntityData(infile);
		}
		else if (line == "[Enemy]") {
			readEntityData(infile);
		}
	}


	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		elapsed += accum;
		glUseProgram(program.programID);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetModelviewMatrix(modelviewMatrix);
		glClearColor(0.9f, 0.0f, 0.0f, 0.0f); 
		glClear(GL_COLOR_BUFFER_BIT);

		if (keys[SDL_SCANCODE_LEFT]){
			if ((entityVect[0]->position.x - (entityVect[0]->sprite.width / 2)) > -4.0f){
				entityVect[0]->acceleration.x -= 1.0f;
			}
		}
		if (keys[SDL_SCANCODE_RIGHT]){
			if ((entityVect[0]->position.x + (entityVect[0]->sprite.width / 2)) < 4.0f){
				entityVect[0]->position.x += 1.0f;
			}
		}
		if (keys[SDL_SCANCODE_SPACE]){
			//player jumps
			entityVect[0]->acceleration.y = 1.5f;
		}

		if (elapsed<FIXED_TIMESTEP){
			accum = elapsed;
			continue;
		}
		while (elapsed >= FIXED_TIMESTEP){
			for (size_t i = 0; i < entityVect.size(); i++){
				entityVect[i]->Update(elapsed);
				if (entityVect[0]->entityType == ENTITY_ENEMY){
					entityVect[0]->CollidesWith(entityVect[1]);
				}
				centerOnPlayer(&program, modelviewMatrix);
				entityVect[i]->Draw(program);
			}
			elapsed -= FIXED_TIMESTEP;
		}
		accum = elapsed;

		//if player collided with enemey end game
		/*
		if (entityVect[0]->collidedLeft = true || entityVect[0]->collidedRight || entityVect[0]->collidedBottom || entityVect[0]->collidedTop){
			exit(0);
		}*/

		program.SetModelviewMatrix(modelviewMatrix);
		renderTileMap(&program, spriteSheetTexture);
		drawCharacters(program, modelviewMatrix, projectionMatrix);
		SDL_GL_SwapWindow(displayWindow);
	}
	
	SDL_Quit();
	return 0;
}