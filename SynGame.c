/* 
	Author: Asrın "Syntriax" Doğan
	Date: 10.09.2019
	Mail: asrindogan99@gmail.com 
*/
#include <stdio.h>
#include <math.h>
#include "allegro-5.0.10-mingw-4.7.0\include\allegro5\allegro.h"
#include "allegro-5.0.10-mingw-4.7.0\include\allegro5\allegro_audio.h"
#include "allegro-5.0.10-mingw-4.7.0\include\allegro5\allegro_acodec.h"
#include "allegro-5.0.10-mingw-4.7.0\include\allegro5\allegro_image.h"
#include "allegro-5.0.10-mingw-4.7.0\include\allegro5\allegro_primitives.h"
#define playerSpeed 7.5
#define initialPlayerHealth 4
#define bulletSpeed 25
#define numberImageSize 5
#define scoreDigitLimit 10
#define enemyLimiter 8
#define initialEnemyLimit 3

typedef struct
{
	float x;
	float y;
} Vector2D;

struct
{
	Vector2D position;
	char health;
	float moveSpeed;
	byte lookDirection;
	int shootPerSecond;
	float shootCooldown;
	unsigned int killedEnemyCount;
	unsigned int score;

	ALLEGRO_BITMAP *playerImage;
} player;

typedef struct
{
	Vector2D position;
	/* char health; */
	float moveSpeed;
	float fireCooldown;
	Vector2D velocity;
} Enemy;

struct 
{
	int enemyLimit;
	int enemyCount;
	Enemy *enemyArray;
} enemies;

typedef struct
{
	char isEnemyBullet;
	Vector2D position;
	Vector2D velocity;
} Bullet;

struct 
{
	int bulletCount;
	Bullet *bulletArray;
} bullets;

void SpawnEnemies();
void CheckBullets();
void RemoveBulletAtIndex(int index);
void RemoveEnemyAtIndex(int index);
void CheckEnemies();
void MoveEnemies();
void InitializeEnemies();
void DestroyGame();
void DrawObject(Vector2D position, ALLEGRO_BITMAP *image, int flag);
void DrawSizedObject(Vector2D position, ALLEGRO_BITMAP *image, int flag, float objectSizeMultiplier);
void DrawScreen();
void DrawScore();
void DrawNumber(Vector2D position, int number);
void Inputs();
void PlayerMovement();
void BulletMovement();
void ShootSoundEffect();
void DieSoundEffect();
void PlayerShoot();
void EnemyShoot();
void BulletCollisions();
void Update();
void DestroyGameWindow();
float VectorMagnitude(Vector2D vector);
float VectorDistance(Vector2D vectorFirst, Vector2D vectorSecond);
byte isVectorExceedingLimits(Vector2D vector, Vector2D limits);
byte CheckCollision(Vector2D *firstPos, Vector2D *secondPos, ALLEGRO_BITMAP *firstMap, ALLEGRO_BITMAP *secondMap);
char InitializeGameWindow();
char InitializeGame();
char DealDamage(char *health);
Vector2D NormalizeVector(Vector2D vector);

ALLEGRO_KEYBOARD_STATE keyboardState;
ALLEGRO_DISPLAY *display;
ALLEGRO_DISPLAY_MODE disp_data;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_COLOR backgroundColor;
ALLEGRO_BITMAP *gameOverImage;

ALLEGRO_SAMPLE *shootSound;
ALLEGRO_SAMPLE_ID shootSoundID;

ALLEGRO_BITMAP *enemyImage;
ALLEGRO_BITMAP *enemyBulletImage;
ALLEGRO_SAMPLE *enemyDieSound;
ALLEGRO_SAMPLE_ID enemyDieSoundID;

ALLEGRO_BITMAP *numberTable;

const char *displayName = "Syn Game";
const Vector2D referenceScreenDimensions = {160, 90};
Vector2D screenDimensions = {0, 0};
Vector2D scorePosition = {0, 0};
Vector2D highScorePosition = {0, 0};
float sizeMultiplier;
float timeFromStart;
const float FPS = 60;
double deltaTime;
unsigned int enemyRespawnCounter = 0;
Vector2D input;
byte isRestart = 0;
byte isRunning = 1;
byte isGameStarted = 0;
byte isGameOver = 0;

void Update()
{
	al_get_keyboard_state(&keyboardState);

	if(al_key_down(&keyboardState, ALLEGRO_KEY_ESCAPE))
		isRunning = 0;

	al_clear_to_color(backgroundColor);

	if(!isGameOver)
	{
		printf("Inputs();\n");
		Inputs();

		if(al_key_down(&keyboardState, ALLEGRO_KEY_F))
			player.score += 1000;
			// enemies.enemyLimit++;

		printf("PlayerMovement();\n");
		PlayerMovement();

		printf("EnemyShoot();\n");
		EnemyShoot();

		printf("BulletCollisions();\n");
		BulletCollisions();

		if(player.shootCooldown < 0.0f)
		{
			if(al_key_down(&keyboardState, ALLEGRO_KEY_SPACE))
			{
				printf("PlayerShoot();\n");
				PlayerShoot();
			}
		}
		else
			player.shootCooldown -= deltaTime;

		timeFromStart += deltaTime;
		player.score = (int)(timeFromStart * timeFromStart) * player.killedEnemyCount;

		if(enemies.enemyLimit != enemyLimiter)
		{
			enemies.enemyLimit = initialEnemyLimit + (int)(timeFromStart / 10);
			if(enemies.enemyCount > enemyLimiter)
				enemies.enemyLimit = enemyLimiter;
		}
	}
	else if(al_key_down(&keyboardState, ALLEGRO_KEY_R))
		isRestart = 1;
	

	printf("CheckEnemies();\n");
	CheckEnemies();
	
	printf("MoveEnemies();\n");
	MoveEnemies();
	
	printf("CheckBullets();\n");
	CheckBullets();

	printf("BulletMovement();\n");
	BulletMovement();

	printf("DrawScreen();\n");
	DrawScreen();
	

	al_flip_display();
}


int main(int argc, char **argv)
{
	if(InitializeGameWindow() == 0)
		return 0;
	do
	{
		if(InitializeGame() == 0)
			return 0;

		while (isRunning && !isRestart)
		{
			ALLEGRO_EVENT ev;
			al_wait_for_event(event_queue, &ev);

			if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				isRunning = 0;

			if(ev.type == ALLEGRO_EVENT_TIMER)
				Update();
		}

		DestroyGame();
	} while (isRestart);

	DestroyGameWindow();
	
	getchar();
	return 0;
}

Vector2D NormalizeVector(Vector2D vector)
{
	Vector2D normalizedVector;
	float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
	
	if(vector.x == 0.0 && vector.y == 0.0)
		return vector;

	normalizedVector.x = vector.x / magnitude;
	normalizedVector.y = vector.y / magnitude;

	return normalizedVector;
}

float VectorMagnitude(Vector2D vector)
{
	return sqrt(vector.x * vector.x + vector.y * vector.y);
}

float VectorDistance(Vector2D vectorFirst, Vector2D vectorSecond)
{
	Vector2D difference;
	difference.x = abs(vectorFirst.x - vectorSecond.x);
	difference.y = abs(vectorFirst.y - vectorSecond.y);
	return VectorMagnitude(difference);
}

byte isVectorExceedingLimits(Vector2D vector, Vector2D limits)
{
	byte result = vector.x > limits.x || vector.x < 0 || vector.y > limits.y || vector.y < 0;
	return result;
}

byte CheckCollision(Vector2D *firstPos, Vector2D *secondPos, ALLEGRO_BITMAP *firstMap, ALLEGRO_BITMAP *secondMap)
{
	Vector2D firstImageSize;
	Vector2D secondImageSize;
	byte result;
	float minDistance;
	float distance;
	firstImageSize.x = (float)al_get_bitmap_width(firstMap);
	firstImageSize.y = (float)al_get_bitmap_height(firstMap);
	secondImageSize.x = (float)al_get_bitmap_width(secondMap);
	secondImageSize.y = (float)al_get_bitmap_height(secondMap);
	
	minDistance = firstImageSize.x > firstImageSize.y ? firstImageSize.y : firstImageSize.x;
	minDistance += secondImageSize.x > secondImageSize.y ? secondImageSize.y : secondImageSize.x;

	minDistance /= 2;
	minDistance *= sizeMultiplier;

	distance = VectorDistance(*firstPos, *secondPos);

	result = distance <= minDistance;
	return result;
}

char InitializeGameWindow()
{
	float x = 0.0f;
	float y = 0.0f;

	if(!al_init() || 
		!al_init_primitives_addon() || 
		!al_init_image_addon() || 
		!al_install_audio() || 
		!al_init_acodec_addon() || 
		!al_reserve_samples(1))
		return 0;

	al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);
	al_set_new_display_flags(ALLEGRO_FULLSCREEN);

	x = disp_data.width;
	y = disp_data.height;
	/* x = 1600; */
	/* y = 900; */

	screenDimensions = (Vector2D){x, y};
	scorePosition = (Vector2D){x * (float)0.05, y * (float)0.05}; 
	highScorePosition = (Vector2D){x * (float)0.05, y * (float)0.95}; 
	sizeMultiplier = screenDimensions.x / referenceScreenDimensions.x;
	display = al_create_display(screenDimensions.x, screenDimensions.y);
	
	if(display == NULL)
		return 0;

	if(!al_install_keyboard())
		return 0;

	deltaTime = 1.0 / FPS;
	timer = al_create_timer(deltaTime);
	event_queue = al_create_event_queue();

	if(event_queue == NULL)
	{
	   printf("Event Queue Error");
	   al_destroy_display(display);
	   al_destroy_timer(timer);
	   return 0;
	}
	
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	
	al_start_timer(timer);
	al_set_window_title(display, displayName);

	backgroundColor.a = 1;
	backgroundColor.a = 0;
	backgroundColor.a = 0;
	backgroundColor.a = 0;
}

char InitializeGame()
{
	shootSound = al_load_sample("Sounds/Shoot.wav");
	enemyDieSound = al_load_sample("Sounds/Die.wav");
	
	InitializeEnemies();

	/* Player Initialization */
	player.position.x = screenDimensions.x / 2;
	player.position.y = screenDimensions.y / 2;
	player.moveSpeed = playerSpeed;
	player.shootPerSecond = 10;
	player.health = initialPlayerHealth;
	player.playerImage = al_load_bitmap("Images/Player.png");
	bullets.bulletCount = 0;
	bullets.bulletArray = (Bullet *) malloc(sizeof(Bullet) * bullets.bulletCount);
	if(player.playerImage == NULL ||
		shootSound == NULL ||
		enemyDieSound == NULL)
		return 0;

	isRunning = 1;
	isRestart = 0;
	isGameOver = 0;
	isGameStarted = 0;
	timeFromStart = 0;
	player.killedEnemyCount = 0;
	return 1;
}

void InitializeEnemies()
{
	int i = 0;
	enemies.enemyLimit = initialEnemyLimit;
	enemies.enemyCount = 0;
	enemies.enemyArray = (Enemy *) malloc(sizeof(Enemy) * enemies.enemyCount);
	enemyImage = al_load_bitmap("Images/Enemy.png");
	numberTable = al_load_bitmap("Images/Numbers.png");
	enemyBulletImage = al_load_bitmap("Images/Bullet.png");
	gameOverImage = al_load_bitmap("Images/GameOver.png");
	SpawnEnemies();
}

void SpawnEnemies()
{
	if(enemyRespawnCounter > 10000)
		enemyRespawnCounter = 0;

	Vector2D enemySpawnVector;
	Vector2D enemyvelocity;
	Enemy *enemy;
	float speed;
	int randomNumber;
	while (enemies.enemyCount < enemies.enemyLimit)
	{
    	srand(time(0) + enemyRespawnCounter);
		enemies.enemyCount++;
		randomNumber = rand() * enemies.enemyCount;
		enemies.enemyArray = (Enemy *) realloc(enemies.enemyArray, sizeof(Enemy) * enemies.enemyCount);
		enemyvelocity.x = (float)(randomNumber % 20000) / 10000;
		enemyvelocity.y = (float)(randomNumber % 2000) / 1000;
		enemyvelocity.x *= randomNumber % 2 == 0 ? -1 : 1;
		enemyvelocity.y *= randomNumber % 4 >= 2  ? -1 : 1;

		speed = (float)(randomNumber % 500 / 100 + 2);
		enemy = (enemies.enemyArray + enemies.enemyCount - 1);
		enemy -> velocity = NormalizeVector(enemyvelocity);
		enemy -> moveSpeed = speed;

		enemySpawnVector.x = enemyvelocity.x > 0 ? 0 : screenDimensions.x;
		enemySpawnVector.y = enemyvelocity.y > 0 ? 0 : screenDimensions.y;

		enemy -> position = enemySpawnVector;
		enemy -> fireCooldown = 01.0 / (rand() % 5) + 2.0;

		enemyRespawnCounter++;
	}
}

void CheckBullets()
{
	int i = 0;
	int j = 0;

	for (; i < bullets.bulletCount; i++)
	{
		if(isVectorExceedingLimits((bullets.bulletArray + i) -> position, screenDimensions))
		{
			for (j = i; j < bullets.bulletCount - 1; j++)
				*(bullets.bulletArray + j) = *(bullets.bulletArray + j + 1);
			
			bullets.bulletCount--;
			bullets.bulletArray = (Bullet *) realloc(bullets.bulletArray, sizeof(Bullet) * bullets.bulletCount);
		}
	}
}

void RemoveBulletAtIndex(int index)
{
	printf("RemoveBulletAtIndex();\n");
	for (; index < bullets.bulletCount - 1; index++)
		*(bullets.bulletArray + index) = *(bullets.bulletArray + index + 1);
	
	bullets.bulletCount--;
	bullets.bulletArray = (Bullet *) realloc(bullets.bulletArray, sizeof(Bullet) * bullets.bulletCount);
}

void RemoveEnemyAtIndex(int index)
{
	printf("RemoveEnemyAtIndex();\n");
	for (; index < enemies.enemyCount - 1; index++)
		*(enemies.enemyArray + index) = *(enemies.enemyArray + index + 1);
	
	enemies.enemyCount--;
	enemies.enemyArray = (Enemy *) realloc(enemies.enemyArray, sizeof(Enemy) * enemies.enemyCount);
	printf("New Enemy Count = %d\n", enemies.enemyCount);
	DieSoundEffect();
}

void CheckEnemies()
{
	int i = 0;

	for (; i < enemies.enemyCount; i++)
		if(isVectorExceedingLimits((enemies.enemyArray + i) -> position, screenDimensions))
			RemoveEnemyAtIndex(i);
			
	SpawnEnemies();
}

void MoveEnemies()
{
	int i = 0;
	Vector2D velocity;
	float speed;

	for (; i < enemies.enemyCount; i++)
	{
		speed = (enemies.enemyArray + i) -> moveSpeed;
		velocity = (enemies.enemyArray + i) -> velocity;
		(enemies.enemyArray + i) -> position.x += velocity.x * speed;
		(enemies.enemyArray + i) -> position.y += velocity.y * speed;
	}
}
void DrawObject(Vector2D position, ALLEGRO_BITMAP *image, int flag)
{
	Vector2D InstantiateSize;
	InstantiateSize.x = (float)al_get_bitmap_width(image);
	InstantiateSize.y = (float)al_get_bitmap_height(image);
	
	al_draw_scaled_bitmap(image,
		0, 0, InstantiateSize.x, InstantiateSize.y,
		position.x - InstantiateSize.x / 2 * sizeMultiplier, position.y - InstantiateSize.y / 2 * sizeMultiplier, 
		InstantiateSize.x * sizeMultiplier, InstantiateSize.y * sizeMultiplier, flag);
}

void DrawNumber(Vector2D position, int number)
{
	Vector2D InstantiateSize;
	float numberFactor;
	InstantiateSize.x = (float)al_get_bitmap_width(numberTable);
	InstantiateSize.y = (float)al_get_bitmap_height(numberTable);
	numberFactor = InstantiateSize.x / 10.0;

	al_draw_scaled_bitmap(numberTable,
		numberFactor * number, 0, numberFactor, InstantiateSize.y,
		position.x - numberFactor / 2 * sizeMultiplier, position.y - InstantiateSize.y / 2 * sizeMultiplier, 
		numberFactor * sizeMultiplier, InstantiateSize.y * sizeMultiplier, 0);
}

void DrawSizedObject(Vector2D position, ALLEGRO_BITMAP *image, int flag, float objectSizeMultiplier)
{
	Vector2D InstantiateSize;
	float sizeFactor = sizeMultiplier * objectSizeMultiplier;
	InstantiateSize.x = (float)al_get_bitmap_width(image);
	InstantiateSize.y = (float)al_get_bitmap_height(image);
	
	al_draw_scaled_bitmap(image,
		0, 0, InstantiateSize.x, InstantiateSize.y,
		position.x - InstantiateSize.x / 2 * sizeFactor, position.y - InstantiateSize.y / 2 * sizeFactor, 
		InstantiateSize.x * sizeFactor, InstantiateSize.y * sizeFactor, flag);
}

void DrawScreen()
{
	int i = 0;
	Vector2D halfScreen = {screenDimensions.x / 2, screenDimensions.y / 2};

	/* Enemy Draw */
	for (i = 0; i < enemies.enemyCount; i++)
		DrawObject((enemies.enemyArray + i) -> position, enemyImage, (enemies.enemyArray + i) -> velocity.x > 0 ? ALLEGRO_FLIP_HORIZONTAL : 0 );
	
	/* Bullet Draw  */
	for (i = 0; i < bullets.bulletCount; i++)
		DrawObject((bullets.bulletArray + i) -> position, enemyBulletImage, 0);
	
	/* Player Draw */
	if(!isGameOver)
		DrawObject(player.position, player.playerImage, player.lookDirection == 1 ? ALLEGRO_FLIP_HORIZONTAL : 0);
	else
		DrawObject(halfScreen, gameOverImage, 0);

	DrawScore();
}

void DrawScore()
{
	unsigned int processedScore = player.score;
	char digit;
	Vector2D spawnPosition;
	int i = scoreDigitLimit;

	/*while (processedScore >= 1 && i > 0)*/
	while (i > 0)
	{
		spawnPosition = scorePosition;
		/* numberImageSize + 1 is because 1 pixel space between digits */
		spawnPosition.x += sizeMultiplier * (numberImageSize + 1) * i;
		digit = processedScore % 10;
		processedScore = (int)(processedScore / 10);
		DrawNumber(spawnPosition, digit);
		i--;
	}
}

void Inputs()
{
	input.x = 0;
	input.y = 0;

	if(al_key_down(&keyboardState, ALLEGRO_KEY_UP))
		input.y = -1;
	else if(al_key_down(&keyboardState, ALLEGRO_KEY_DOWN))
		input.y = 1;

	if(al_key_down(&keyboardState, ALLEGRO_KEY_RIGHT))
	{
		input.x = 1;
		player.lookDirection = input.x;
	}
	else if(al_key_down(&keyboardState, ALLEGRO_KEY_LEFT))
	{
		input.x = -1;
		player.lookDirection = input.x;
	}


	input =	NormalizeVector(input);
}

void PlayerMovement()
{
	player.position.x += input.x * player.moveSpeed;
	player.position.y += input.y * player.moveSpeed;
}

char DealDamage(char *health)
{
	return --*health <= 0;
}

void BulletMovement()
{
	Bullet *bullet;
	int i = 0;
	for (; i < bullets.bulletCount; i++)
	{
		bullet = (bullets.bulletArray + i);
		bullet -> position.x += bullet -> velocity.x; 
		bullet -> position.y += bullet -> velocity.y; 
	}
}

void ShootSoundEffect()
{
	printf("ShootSoundEffect();\n");
	al_stop_sample(&shootSoundID);
	al_play_sample(shootSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &shootSoundID);
}

void DieSoundEffect()
{
	printf("DieSoundEffect();\n");
	al_stop_sample(&enemyDieSoundID);
	al_play_sample(enemyDieSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &enemyDieSoundID);
}

void PlayerShoot()
{
	Vector2D shootDir;
	Bullet *newBullet;
	float offset = (al_get_bitmap_width(player.playerImage) + al_get_bitmap_width(enemyBulletImage) * 2.0 * sizeMultiplier);
	
	if(player.lookDirection != 1)
		offset = -offset;

	shootDir.x = player.lookDirection == 1 ? bulletSpeed : -bulletSpeed;
	shootDir.y = 0;
	
	player.shootCooldown = 1 / (float)player.shootPerSecond;
	bullets.bulletCount++;
	bullets.bulletArray = (Bullet *) realloc(bullets.bulletArray, sizeof(Bullet) * bullets.bulletCount);
	newBullet = (bullets.bulletArray + bullets.bulletCount - 1);
	newBullet -> position = player.position;
	newBullet -> position.x += offset; 
	newBullet -> velocity = shootDir; 
	newBullet -> isEnemyBullet = 0; 

	ShootSoundEffect();
}

void EnemyShoot()
{
	Vector2D shootDir;
	Vector2D normalizedVec;
	Enemy *enemy;
	Bullet *bullet;
	int i;
	float offset = (al_get_bitmap_width(player.playerImage) + al_get_bitmap_width(enemyBulletImage) * 2.0 * sizeMultiplier);

	for (i = 0; i < enemies.enemyCount; i++)
	{
		srand(time(0) + enemyRespawnCounter++);
		enemy = (enemies.enemyArray + i);
		if(enemy -> fireCooldown > 0.0)
			enemy -> fireCooldown -= deltaTime;
		else
		{
			shootDir = (Vector2D){player.position.x - enemy -> position.x , player.position.y - enemy -> position.y};
			
			shootDir = NormalizeVector(shootDir);
			normalizedVec = shootDir;
			shootDir = (Vector2D){shootDir.x * bulletSpeed, shootDir.y * bulletSpeed};

			enemy -> fireCooldown = 1.0 / (rand() % 5) + 2.0;
			bullets.bulletCount++;
			bullets.bulletArray = (Bullet *) realloc(bullets.bulletArray, sizeof(Bullet) * bullets.bulletCount);
			bullet = (bullets.bulletArray + bullets.bulletCount - 1);
			bullet -> position = enemy -> position;
			bullet -> position.x += normalizedVec.x * offset; 
			bullet -> position.y += normalizedVec.y * offset; 
			bullet -> velocity = shootDir; 
			bullet -> isEnemyBullet = 1; 
			ShootSoundEffect();
		}
	}

}

void BulletCollisions()
{
	Bullet *bullet;
	Enemy *enemy;
	int bulletCounter = 0;
	int enemyCounter = 0;
	int i = 0;
	printf("Enemy-Bullet\n");
	for (enemyCounter = 0; enemyCounter < enemies.enemyCount; enemyCounter++)
	{
		printf("Enemy-Bullet|enemyCounter\n");
		enemy = (enemies.enemyArray + enemyCounter);
		for (bulletCounter = 0; bulletCounter < bullets.bulletCount; bulletCounter++)
		{
			bullet = (bullets.bulletArray + bulletCounter);
			printf("Player-Bullet\n");
			if(bullet -> isEnemyBullet == 1 && CheckCollision(
					&bullet -> position,
					&player.position,
					enemyBulletImage,
					player.playerImage
				))
			{
				RemoveBulletAtIndex(bulletCounter);
				bulletCounter--;

				if(DealDamage(&player.health))
					isGameOver = 1;
				continue;
			}
		
			if(bullet -> isEnemyBullet == 0 && CheckCollision(
				&bullet -> position,
				&enemy -> position,
				enemyBulletImage,
				enemyImage
			))
			{
				printf("Enemy-Bullet|EnemyRemove\n");
				RemoveEnemyAtIndex(enemyCounter);
				enemyCounter--;

				printf("Enemy-Bullet|BulletRemove\n");
				RemoveBulletAtIndex(bulletCounter);
				bulletCounter--;
				player.killedEnemyCount++;
			}
		}

		printf("Enemy-Player\n");
		if(CheckCollision(
				&enemy -> position,
				&player.position,
				enemyImage,
				player.playerImage
			))
		{				
			RemoveEnemyAtIndex(enemyCounter);
			enemyCounter--;

			if(DealDamage(&player.health))
				isGameOver = 1;
		}
	}
	
}

void DestroyGame()
{
	al_destroy_bitmap(enemyImage);
	al_destroy_bitmap(enemyBulletImage);
	al_destroy_bitmap(gameOverImage);
	al_destroy_bitmap(numberTable);
	al_destroy_sample(shootSound);
	al_destroy_sample(enemyDieSound);
	al_destroy_bitmap(player.playerImage);
	free(enemies.enemyArray);
}

void DestroyGameWindow()
{
	al_destroy_display(display);
	al_uninstall_keyboard();
}