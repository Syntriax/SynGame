/* 
    Author: Asrın "Syntriax" Doğan
    Start: 05.09.2019
    Mail: asrindogan99@gmail.com 

    Simple Shoot 'Em Up game. 

    Keys:
    Move Up - Up Arrow
    Move Right - Right Arrow
    Move Down - Down Arrow
    Move Left - Left Arrow
    Shoot - Space
    Restart - R
    Exit - Escape(ESC)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allegro\include\allegro5\allegro.h"
#include "allegro\include\allegro5\allegro_audio.h"
#include "allegro\include\allegro5\allegro_acodec.h"
#include "allegro\include\allegro5\allegro_image.h"
#include "allegro\include\allegro5\allegro_primitives.h"

#define playerSpeed 0.75
#define enemySpeed 0.1
#define initialPlayerHealth 4
#define bulletSpeed 2.5
#define numberImageSize 5
#define scoreDigitLimit 11
#define timerDigitLimit 4
#define initialEnemyLimit 3
#define sampleCount 3
#define colon 10
#define numbersLength 11

typedef struct
{
    float x;
    float y;
} Vector2D;

typedef struct
{
    Vector2D originalSize;
    Vector2D size;
    ALLEGRO_BITMAP *bitmap;
} Image;

typedef struct
{
    Vector2D position;
    /* char health; */
    float moveSpeed;
    float fireCooldown;
    Vector2D velocity;
} Enemy;

typedef struct
{
    char isEnemyBullet;
    Vector2D position;
    Vector2D velocity;
} Bullet;

struct 
{
    int enemyLimit;
    int enemyCount;
    Enemy *enemyArray;
} enemies;

struct 
{
    int bulletCount;
    Bullet *bulletArray;
} bullets;

struct
{
    Vector2D position;
    char health;
    float moveSpeed;
    char lookDirection;
    int shootPerSecond;
    float shootCooldown;
    unsigned int killedEnemyCount;
    unsigned int score;

    Image image;
} player;

char InitializeGameWindow();
char InitializeGame();
char InitializeEnemies();
char DealDamage(char *health);
char isVectorExceedingLimits(Vector2D vector, Vector2D limits);
char CheckCollision(Vector2D *firstPos, Vector2D *secondPos, Image *firstMap, Image *secondMap);

void Update();
void SpawnEnemies();
void CheckBullets();
void RemoveBulletAtIndex(int index);
void RemoveEnemyAtIndex(int index);
void CheckEnemies();
void MoveEnemies();
void LimitEnemies();
void DrawObject(Vector2D position, Image *image, int flag);
void DrawNumber(Vector2D position, int number);
void DrawSizedObject(Vector2D position, Image *image, int flag, float objectscreenSizeMultiplier);
void DrawScreen();
void DrawScore();
void DrawHighScore();
void DrawTimer();
void CheckHighScore();
void GetHighScore();
void GetSettings();
void CalculateScore();
void Inputs();
void PlayerMovement();
void ClampPlayerPositionToScreenDimensions();
void ShootSoundEffect();
void DieSoundEffect();
void PlayerShootCheck();
void PlayerShoot();
void EnemyShoot();
void BulletMovement();
void BulletCollisions();
void DestroyGame();
void DestroyGameWindow();


float VectorMagnitude(Vector2D vector);
float VectorDistanceBetween(Vector2D vectorFirst, Vector2D vectorSecond);

Vector2D NormalizeVector(Vector2D vector);

Image InitImage(const char *path);

ALLEGRO_KEYBOARD_STATE keyboardState;
ALLEGRO_DISPLAY_MODE disp_data;
ALLEGRO_COLOR backgroundColor;

ALLEGRO_DISPLAY *display            = NULL;
ALLEGRO_EVENT_QUEUE *event_queue    = NULL;
ALLEGRO_TIMER *timer                = NULL;

Image gameOverImage;

ALLEGRO_SAMPLE *BGM = NULL;

ALLEGRO_SAMPLE *shootSound = NULL;
ALLEGRO_SAMPLE_ID shootSoundID;

Image enemyImage;
Image enemyBulletImage;
ALLEGRO_SAMPLE *enemyDieSound = NULL;
ALLEGRO_SAMPLE_ID enemyDieSoundID;

Image numberTable;

const char *playerImagePath     = "Images/Player.png";
const char *enemyImagePath      = "Images/Enemy.png";
const char *bulletImagePath     = "Images/Bullet.png";
const char *gameOverImagePath   = "Images/GameOver.png";
const char *numbersImagePath    = "Images/Numbers.png";

const char *dieSoundPath    = "Sounds/Die.wav";
const char *bgmSoundPath    = "Sounds/Background.wav";
const char *shootSoundPath  = "Sounds/Shoot.wav";

const char *displayName     = "Syn Game";
const char *savePath        = "Save.syn";
const char *settingsPath    = "Settings.syn";
const char *settingsFormat  = "%d\n%d\n%d";

int isFullscreen = 1;
int settingsWidth = 1600;
int settingsHeight = 900;

const Vector2D referenceScreenDimensions = {160, 90};
Vector2D screenDimensions   = {0, 0};
Vector2D scorePosition      = {0, 0};
Vector2D highScorePosition  = {0, 0};
Vector2D timerPosition      = {0, 0};

float screenSizeMultiplier; 
float timeSinceStart;

const float FPS = 60;

double deltaTime;

unsigned int enemyRespawnCounter = 0;
unsigned int highScore = 0;

int enemyLimiter = 12;

Vector2D input;

char isRestart  = 0;
char isRunning  = 1;
char isGameOver = 0;

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

        printf("PlayerMovement();\n");
        PlayerMovement();

        printf("EnemyShoot();\n");
        EnemyShoot();

        printf("BulletCollisions();\n");
        BulletCollisions();

        printf("PlayerShootCheck();\n");
        PlayerShootCheck();

        printf("LimitEnemies();\n");
        LimitEnemies();

        timeSinceStart += deltaTime;

        printf("CalculateScore();\n");
        CalculateScore();
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
    {
        DestroyGameWindow();
        getchar();
        return 0;
    }
        
    do
    {
        if(InitializeGame() == 0)
        {
            DestroyGame();
            getchar();
            return 0;
        }

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
    } 
    while (isRestart);

    DestroyGameWindow();
        
    getchar();
    return 0;
}

Image InitImage(const char *path)
{
    Image result;
    result.bitmap = al_load_bitmap(path);

    result.originalSize.x = al_get_bitmap_width(result.bitmap);
    result.originalSize.y = al_get_bitmap_height(result.bitmap);

    result.size.x = result.originalSize.x * screenSizeMultiplier;
    result.size.y = result.originalSize.y * screenSizeMultiplier;
        
    return result;
}

Vector2D NormalizeVector(Vector2D vector)
{
    Vector2D normalizedVector;
    float magnitude;

    magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
        
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

float VectorDistanceBetween(Vector2D vectorFirst, Vector2D vectorSecond)
{
    Vector2D difference;
    difference.x = abs(vectorFirst.x - vectorSecond.x);
    difference.y = abs(vectorFirst.y - vectorSecond.y);
    return VectorMagnitude(difference);
}

char isVectorExceedingLimits(Vector2D vector, Vector2D limits)
{
    char result = vector.x > limits.x || vector.x < 0 || vector.y > limits.y || vector.y < 0;
    return result;
}

/* 
    Gets the shortest dimensions of both images, sums these dimension and divides by 2 to get minimum accepted distance.
    And compares the distance between those objects to the minimum distancce to check if they're colliding.
    It's the most simple and optimized way I can think of for a game like this.
*/
char CheckCollision(Vector2D *firstPos, Vector2D *secondPos, Image *firstMap, Image *secondMap)
{
    char result;
    float minDistance;
    float distance;
        
    minDistance = firstMap -> size.x > firstMap -> size.y ? 
        firstMap -> size.y : 
        firstMap -> size.x;

    minDistance += secondMap -> size.x > secondMap -> size.y ? 
        secondMap -> size.y : 
        secondMap -> size.x;

    minDistance *= 0.5f;

    distance = VectorDistanceBetween(*firstPos, *secondPos);

    result = distance <= minDistance;
    return result;
}

char InitializeGameWindow()
{
    float x;
    float y;
    x = 0.0f;
    y = 0.0f;

    if(!al_init() || 
        !al_init_primitives_addon() || 
        !al_init_image_addon() || 
        !al_install_audio() || 
        !al_init_acodec_addon() || 
        !al_reserve_samples(sampleCount))
        return 0;

    GetSettings();

    if(isFullscreen == 1)
    {
        al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);
        al_set_new_display_flags(ALLEGRO_FULLSCREEN);

        x = disp_data.width;
        y = disp_data.height;
    }
    else
    {
        x = settingsWidth;
        y = settingsHeight;
    }

    screenDimensions    = (Vector2D){x, y};

    scorePosition       = (Vector2D){x * (float)0.05, y * (float)0.05}; /* Upper Left Position */
    highScorePosition   = (Vector2D){x * (float)0.95, y * (float)0.05}; /* Upper Right Position */
    timerPosition       = (Vector2D){x * (float)0.5 , y * (float)0.95}; /* Bottom Center Position */

    screenSizeMultiplier = screenDimensions.x / referenceScreenDimensions.x;
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
    backgroundColor.r = .1;
    backgroundColor.g = .1;
    backgroundColor.b = .1;
        
    /* BGM is an exception since I don't want to it to restart itself every restart */
    BGM = al_load_sample(bgmSoundPath);
        
    if(BGM == NULL)
        return 0;
        
    al_play_sample(BGM, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    return 1;
}

char InitializeGame()
{
    shootSound      = al_load_sample(shootSoundPath);
    enemyDieSound   = al_load_sample(dieSoundPath);

    if(shootSound == NULL || enemyDieSound == NULL)
        return 0;
        
    if(!InitializeEnemies())
        return 0;
    GetHighScore();

    /* Player Initialization */
    player.position.x = screenDimensions.x / 2;
    player.position.y = screenDimensions.y / 2;
    player.moveSpeed = playerSpeed * screenSizeMultiplier;
    player.shootPerSecond = 10;
    player.health = initialPlayerHealth;
    player.image = InitImage(playerImagePath);
        
    if(player.image.bitmap == NULL ||
        shootSound == NULL ||
        enemyDieSound == NULL)
        return 0;

        
    bullets.bulletCount = 0;
    bullets.bulletArray = (Bullet *) malloc(sizeof(Bullet) * bullets.bulletCount);

    /* Game Initialization */
    isRunning = 1;
    isRestart = 0;
    isGameOver = 0;
    timeSinceStart = 0;
    player.killedEnemyCount = 0;
    return 1;
}

char InitializeEnemies()
{
    enemies.enemyLimit = initialEnemyLimit;
    enemies.enemyCount = 0;
    enemies.enemyArray = (Enemy *) malloc(sizeof(Enemy) * enemies.enemyCount);

    enemyImage          = InitImage(enemyImagePath);
    numberTable         = InitImage(numbersImagePath);
    enemyBulletImage    = InitImage(bulletImagePath);
    gameOverImage       = InitImage(gameOverImagePath);
        
    if( enemyImage.bitmap       == NULL ||
        numberTable.bitmap      == NULL ||
        enemyBulletImage.bitmap == NULL ||
        gameOverImage.bitmap    == NULL )
    return 0;

    SpawnEnemies();
    return 1;
}

char DealDamage(char *health)
{
    return --*health <= 0;
}

void SpawnEnemies()
{
    Vector2D enemySpawnVector;
    Vector2D enemyVelocity;
    Enemy *enemy;
    float speed;
    int randomNumber;

    if(enemyRespawnCounter > 10000)
        enemyRespawnCounter = 0;

    while (enemies.enemyCount < enemies.enemyLimit)
    {
        /* enemyRespawnCounter is just for making the value of rand() more randomized */
        srand(time(0) + enemyRespawnCounter);
        randomNumber = rand();
        enemies.enemyCount++;
        enemies.enemyArray = (Enemy *) realloc(enemies.enemyArray, sizeof(Enemy) * enemies.enemyCount);

        /* Randomizing the velocity */
        enemyVelocity.x = (float)(randomNumber % 20000) / 10000;
        enemyVelocity.y = (float)(randomNumber % 2000) / 1000;
        enemyVelocity.x *= randomNumber % 2 == 0 ? -1 : 1;
        enemyVelocity.y *= randomNumber % 4 >= 2  ? -1 : 1;

        speed = (float)(randomNumber % 500 / 100 + 2) * enemySpeed;
        enemy = (enemies.enemyArray + enemies.enemyCount - 1);
        enemy -> velocity = NormalizeVector(enemyVelocity);
        enemy -> moveSpeed = speed * screenSizeMultiplier;

        enemySpawnVector.x = enemyVelocity.x > 0 ? 0 : screenDimensions.x;
        enemySpawnVector.y = enemyVelocity.y > 0 ? 0 : screenDimensions.y;

        enemy -> position = enemySpawnVector;
        enemy -> fireCooldown = 1.0 / (rand() % 5) + 2.0;

        enemyRespawnCounter++;
    }
}

void CheckBullets()
{
    int i;
    int j;

    for (i = 0; i < bullets.bulletCount; i++)
        if(isVectorExceedingLimits((bullets.bulletArray + i) -> position, screenDimensions))
        {
            for (j = i; j < bullets.bulletCount - 1; j++)
                *(bullets.bulletArray + j) = *(bullets.bulletArray + j + 1);
            
            bullets.bulletCount--;
            bullets.bulletArray = (Bullet *) realloc(bullets.bulletArray, sizeof(Bullet) * bullets.bulletCount);
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

    DieSoundEffect();
}

void CheckEnemies()
{
    int i;

    for (i = 0; i < enemies.enemyCount; i++)
        if(isVectorExceedingLimits((enemies.enemyArray + i) -> position, screenDimensions))
            RemoveEnemyAtIndex(i);
            
    SpawnEnemies();
}

void MoveEnemies()
{
    Vector2D velocity;
    float speed;
    int i;

    for (i = 0; i < enemies.enemyCount; i++)
    {
        speed = (enemies.enemyArray + i) -> moveSpeed;
        velocity = (enemies.enemyArray + i) -> velocity;
        (enemies.enemyArray + i) -> position.x += velocity.x * speed;
        (enemies.enemyArray + i) -> position.y += velocity.y * speed;
    }
}

void LimitEnemies()
{
    if(enemies.enemyLimit != enemyLimiter)
    {
        enemies.enemyLimit = initialEnemyLimit + (int)(timeSinceStart / 10);
        
        if(enemies.enemyCount > enemyLimiter)
            enemies.enemyLimit = enemyLimiter;
    }
}

void DrawObject(Vector2D position, Image *image, int flag)
{
    Vector2D InstantiateSize;
    Vector2D originalSize;

    InstantiateSize = image -> size;
    originalSize = image -> originalSize;

    al_draw_scaled_bitmap(image -> bitmap,
        0, 0, originalSize.x, originalSize.y,
        position.x - InstantiateSize.x / 2, position.y - InstantiateSize.y / 2, 
        InstantiateSize.x, InstantiateSize.y, flag);
}

/* 
    Allegra Fonts is not working so I use this for displaying numbers 
    Special Characters:
    10 = : 
*/
void DrawNumber(Vector2D position, int number)
{
    Vector2D InstantiateSize;
    Vector2D originalSize;

    InstantiateSize = numberTable.size; 
    originalSize = numberTable.originalSize;

    al_draw_scaled_bitmap(numberTable.bitmap,
        numberImageSize * number, 0, numberImageSize, originalSize.y,
        position.x - numberImageSize / 2 * screenSizeMultiplier, position.y - InstantiateSize.y / 2, 
        numberImageSize * screenSizeMultiplier, InstantiateSize.y, 0);
}

void DrawSizedObject(Vector2D position, Image *image, int flag, float objectscreenSizeMultiplier)
{
    Vector2D InstantiateSize;
    float sizeFactor;

    sizeFactor = screenSizeMultiplier * objectscreenSizeMultiplier;
    InstantiateSize = image -> size;
        
    al_draw_scaled_bitmap(image -> bitmap,
        0, 0, InstantiateSize.x, InstantiateSize.y,
        position.x - InstantiateSize.x / 2 * sizeFactor, position.y - InstantiateSize.y / 2 * sizeFactor, 
        InstantiateSize.x * sizeFactor, InstantiateSize.y * sizeFactor, flag);
}

void DrawScreen()
{
    int i;
    Vector2D halfScreen;
    halfScreen = (Vector2D){screenDimensions.x / 2, screenDimensions.y / 2};

    /* Enemy Draw */
    for (i = 0; i < enemies.enemyCount; i++)
        DrawObject((enemies.enemyArray + i) -> position, &enemyImage, (enemies.enemyArray + i) -> velocity.x > 0 ? ALLEGRO_FLIP_HORIZONTAL : 0 );
        
    /* Bullet Draw  */
    for (i = 0; i < bullets.bulletCount; i++)
        DrawObject((bullets.bulletArray + i) -> position, &enemyBulletImage, 0);
        
    /* Player Draw */
    if(!isGameOver)
        DrawObject(player.position, &player.image, player.lookDirection == 1 ? ALLEGRO_FLIP_HORIZONTAL : 0);
    else
        DrawObject(halfScreen, &gameOverImage, 0);

    DrawScore();
    DrawHighScore();
    DrawTimer();
}

void DrawScore()
{
    unsigned int processedScore;
    char digit;
    Vector2D spawnPosition;
    int i;

    processedScore = player.score;

    for(i = scoreDigitLimit - 1; i >= 0; i--)
    {
        spawnPosition = scorePosition;
        /* numberImageSize + 1 is because 1 pixel space between digits */
        spawnPosition.x += screenSizeMultiplier * (numberImageSize + 1) * i;
        digit = processedScore % 10;
        processedScore = (int)(processedScore / 10);
        DrawNumber(spawnPosition, digit);
    }
}

void DrawHighScore()
{
    unsigned int processedScore;
    char digit;
    Vector2D spawnPosition;
    int i;

    processedScore = highScore;

    for(i = 0; i < scoreDigitLimit; i++)
    {
        spawnPosition = highScorePosition;
        /* numberImageSize + 1 is because 1 pixel space between digits */
        spawnPosition.x -= screenSizeMultiplier * (numberImageSize + 1) * i;
        digit = processedScore % 10;
        processedScore = (int)(processedScore / 10);
        DrawNumber(spawnPosition, digit);
    }
}

void DrawTimer()
{
    int seconds;
    int minutes;
    char digit;
    Vector2D spawnPosition;
    int i;

    seconds = (int)timeSinceStart % 60;
    minutes = (timeSinceStart - seconds) / 60;
    spawnPosition = timerPosition;
    i = -timerDigitLimit / 2;

    while (i < 0)
    {
        /* numberImageSize + 1 is because 1 pixel space between digits */
        spawnPosition.x = timerPosition.x - screenSizeMultiplier * (numberImageSize + 1) * i;
        
        digit = seconds % 10;
        seconds = (int)(seconds / 10);
        DrawNumber(spawnPosition, digit);
        i++;
    }

    /* numberImageSize + 1 is because 1 pixel space between digits */
    spawnPosition.x = timerPosition.x - screenSizeMultiplier * (numberImageSize + 1) * i;
    DrawNumber(spawnPosition, colon);
    i++;
        
    while (i < (timerDigitLimit / 2) + 1)
    {
        /* numberImageSize + 1 is because 1 pixel space between digits */
        spawnPosition.x = timerPosition.x - screenSizeMultiplier * (numberImageSize + 1) * i;
        
        if(i == 0)
        {
            DrawNumber(spawnPosition, colon);
            i++;
            continue;
        }
        
        digit = minutes % 10;
        minutes = (int)(minutes / 10);
        DrawNumber(spawnPosition, digit);
        i++;
    }
}

void CheckHighScore()
{
    FILE *saveFile;
    printf("Checking Highscore = %d and Score = %d\n", highScore, player.score);
        
    if(player.score < highScore)
        return;

    saveFile = fopen(savePath, "wb");
    if(saveFile == NULL)
    {
        printf("!!!!Error Saving Highscore!!!!\n");
        return;
    }

    highScore = player.score;
    fwrite(&highScore, sizeof(highScore), 1, saveFile);
    fclose(saveFile);
}

void GetHighScore()
{
    FILE *saveFile = fopen(savePath, "rb");
    printf("Getting Highscore\n");
    if(saveFile == NULL)
    {
        printf("!!!!Error Reading Highscore!!!!\n");
        highScore = 0;
        return;
    }

    fread(&highScore, sizeof(highScore), 1, saveFile);
    fclose(saveFile);
}

void GetSettings()
{
    FILE *settingsFile = fopen(settingsPath, "r");
    printf("Getting Settings\n");
    if(settingsFile == NULL)
    {
        printf("!!!!Error Reading Settings!!!!\n");
        settingsFile = fopen(settingsPath, "w");
        if(settingsFile == NULL)
            printf("!!!!Error Creating Settings!!!!\n");
        else
        {
            fprintf(settingsFile, settingsFormat, isFullscreen, settingsWidth, settingsHeight, enemyLimiter);
            fclose(settingsFile);
        }
        return;
    }

    fscanf(settingsFile, settingsFormat, &isFullscreen, &settingsWidth, &settingsHeight, &enemyLimiter);
    fclose(settingsFile);
}

void CalculateScore()
{
    player.score = (int)(timeSinceStart * timeSinceStart) * (player.killedEnemyCount + 1);
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

    input = NormalizeVector(input);
}

void PlayerMovement()
{
    player.position.x += input.x * player.moveSpeed;
    player.position.y += input.y * player.moveSpeed;

    ClampPlayerPositionToScreenDimensions();
}

void ClampPlayerPositionToScreenDimensions()
{
    if(player.position.x < 0)
        player.position.x = 0;
    else if(player.position.x > screenDimensions.x)
        player.position.x = screenDimensions.x;

    if(player.position.y < 0)
        player.position.y = 0;
    else if(player.position.y > screenDimensions.y)
        player.position.y = screenDimensions.y;
}

void ShootSoundEffect()
{
    printf("ShootSoundEffect();\n");
    /* al_stop_sample(&shootSoundID); */
    al_play_sample(shootSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &shootSoundID);
}

void DieSoundEffect()
{
    printf("DieSoundEffect();\n");
    /* al_stop_sample(&enemyDieSoundID); */
    al_play_sample(enemyDieSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &enemyDieSoundID);
}

void PlayerShootCheck()
{
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
}

void PlayerShoot()
{
    Vector2D shootDir;
    Bullet *newBullet;
    float offset;

    offset = (player.image.size.x + enemyBulletImage.size.x * 2.0);
        
    if(player.lookDirection != 1)
        offset = -offset;

    shootDir.x = player.lookDirection == 1 ? bulletSpeed : -bulletSpeed;
    shootDir.x *= screenSizeMultiplier;
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
    float offset;

    offset = (player.image.size.x + enemyBulletImage.size.x * 2.0);

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
            shootDir = (Vector2D){shootDir.x * bulletSpeed * screenSizeMultiplier, shootDir.y * bulletSpeed * screenSizeMultiplier};

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
    int bulletCounter;
    int enemyCounter;

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
                    &enemyBulletImage,
                    &player.image
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
                &enemyBulletImage,
                &enemyImage
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
                &enemyImage,
                &player.image
            ))
        {               
            RemoveEnemyAtIndex(enemyCounter);
            enemyCounter--;

            if(DealDamage(&player.health))
                isGameOver = 1;
        }
    }
}

void BulletMovement()
{
    Bullet *bullet;
    int i;
    for (i = 0; i < bullets.bulletCount; i++)
    {
        bullet = (bullets.bulletArray + i);
        bullet -> position.x += bullet -> velocity.x; 
        bullet -> position.y += bullet -> velocity.y; 
    }
}

void DestroyGame()
{
    CheckHighScore();
    al_destroy_bitmap(enemyImage.bitmap);
    al_destroy_bitmap(enemyBulletImage.bitmap);
    al_destroy_bitmap(gameOverImage.bitmap);
    al_destroy_bitmap(numberTable.bitmap);
    al_destroy_bitmap(player.image.bitmap);
    al_destroy_sample(shootSound);
    al_destroy_sample(enemyDieSound);
    free(enemies.enemyArray);
}

void DestroyGameWindow()
{
    /* BGM is an exception since I don't want to it to restart itself every restart */
    al_destroy_sample(BGM);
    al_destroy_display(display);
    al_uninstall_keyboard();
}