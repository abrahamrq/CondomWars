// Oscar Abraham Rodriguez Quintanilla
// A01195653
// Tania Garrido Salido
// A0
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <math.h>
#include <algorithm>
#include "imageLoader.h"
#include "glm/glm.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2_Mixer/SDL_Mixer.h>

////////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <fstream>

using namespace std;

//Amount of models and model ids
#define MODEL_COUNT 10
#define LEIA_MOD 0
#define HAN_MOD 1
#define CONDOM_MOD 2
#define SPERM_MOD 3
#define R2_MOD 4
//#define SOAP_2_MOD 3
//#define TOOTHPASTE_MOD 4

GLMmodel models[MODEL_COUNT];
bool leia = true;
bool gameover = false;
//The music that will be played
Mix_Music *gMusic = NULL;

//The sound effects that will be used
Mix_Music *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;


Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h)
{
    
}

Image::~Image()
{
    delete[] pixels;
}

namespace
{
    //Converts a four-character array to an integer, using little-endian form
    int toInt(const char* bytes)
    {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }
    
    //Converts a two-character array to a short, using little-endian form
    short toShort(const char* bytes)
    {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }
    
    //Reads the next four bytes as an integer, using little-endian form
    int readInt(ifstream &input)
    {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }
    
    //Reads the next two bytes as a short, using little-endian form
    short readShort(ifstream &input)
    {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }
    
    //Just like auto_ptr, but for arrays
    template<class T>
    class auto_array
    {
    private:
        T* array;
        mutable bool isReleased;
    public:
        explicit auto_array(T* array_ = NULL) :
        array(array_), isReleased(false)
        {
        }
        
        auto_array(const auto_array<T> &aarray)
        {
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        ~auto_array()
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
        }
        
        T* get() const
        {
            return array;
        }
        
        T &operator*() const
        {
            return *array;
        }
        
        void operator=(const auto_array<T> &aarray)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        T* operator->() const
        {
            return array;
        }
        
        T* release()
        {
            isReleased = true;
            return array;
        }
        
        void reset(T* array_ = NULL)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = array_;
        }
        
        T* operator+(int i)
        {
            return array + i;
        }
        
        T &operator[](int i)
        {
            return array[i];
        }
    };
}

Image* loadBMP(const char* filename)
{
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"Could not find file");
    char buffer[2];
    input.read(buffer, 2);
    assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
    input.ignore(8);
    int dataOffset = readInt(input);
    
    //Read the header
    int headerSize = readInt(input);
    int width;
    int height;
    switch (headerSize)
    {
        case 40:
            //V3
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            assert(readShort(input) == 0 || !"Image is compressed");
            break;
        case 12:
            //OS/2 V1
            width = readShort(input);
            height = readShort(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            break;
        case 64:
            //OS/2 V2
            assert(!"Can't load OS/2 V2 bitmaps");
            break;
        case 108:
            //Windows V4
            assert(!"Can't load Windows V4 bitmaps");
            break;
        case 124:
            //Windows V5
            assert(!"Can't load Windows V5 bitmaps");
            break;
        default:
            assert(!"Unknown bitmap format");
    }
    
    //Read the data
    int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
    int size = bytesPerRow * height;
    auto_array<char> pixels(new char[size]);
    input.seekg(dataOffset, ios_base::beg);
    input.read(pixels.get(), size);
    
    //Get the data into the right format
    auto_array<char> pixels2(new char[width * height * 3]);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                pixels2[3 * (width * y + x) + c] =
                pixels[bytesPerRow * y + 3 * x + (2 - c)];
            }
        }
    }
    
    input.close();
    return new Image(pixels2.release(), width, height);
}
////////////////////////////////////////////////////////////////////////////////

int WIDTH = 500;
int HEIGHT = 500;

///////////////////////////////////CLASSES//////////////////////////////////////
class Player{
private:
    int score;
    float xPos;
    float yPos;
    std::string name;
    int lives;
    
public:
    Player(){
        lives = 1;
        score = 0;
        name = "";
    }
    
    Player(int score, std::string name, int lives){
        this->score = score;
        this->name = name;
        this->lives = lives;
        xPos = 0;
        yPos = -3.5;
    }
    
    float getXPos(){
        return this->xPos;
    }
    
    float getYPos(){
        return this->yPos;
    }
    
    int getScore(){
        return this->score;
    }
    
    void setScore(int score){
        this->score = score;
    }
    
    int getLives(){
        return this->lives;
    }
    
    void setLives(int lives){
        this->lives = lives;
    }
    
    std::string getName(){
        return this->name;
    }
    
    void setName(std::string name){
        this->name = name;
    }
    
    void display(){
        glPushMatrix();
        glColor3f(0, 0, 0);
        glTranslated(this->xPos, this->yPos, 0);
        glScalef(.5, .5, 0.1);
        if (leia){
            glmDraw(&models[LEIA_MOD], GLM_COLOR | GLM_FLAT);
        } else {
            glmDraw(&models[HAN_MOD], GLM_COLOR | GLM_FLAT);
        }
        glPopMatrix();
    }
    
    void move_right(){
        if (!((xPos + .01) > 3.9)){
            this->xPos = this->xPos + 0.1;
        }
    }
    
    void move_left(){
        if (!((xPos - .01) < -3.9)){
            this->xPos = this->xPos - 0.1;
        }
    }
    
    void reset(){
        this->score = 0;
        xPos = 0;
        yPos = -3;
    }
};

Player player;

class Object{
private:
    int type;
    float xPos;
    float yPos;
    
public:
    Object(){
        type = 1;
        xPos = 0;
        yPos = 0;
    }
    
    Object(int type, float xPos){
        this->type = type;
        this->xPos = xPos;
        this->yPos = 3;
    }
    
    int getType(){
        return this->type;
    }
    
    void setType(int type){
        this->type = type;
    }
    
    void display(){
        glPushMatrix();
        glColor3f(5, 5, 0); 
        glTranslated(this->xPos, this->yPos, 0);
        glScalef(.5, .5, 0.1);
        if (this->type == 0){
            glmDraw(&models[CONDOM_MOD], GLM_COLOR | GLM_FLAT);
        } else if (this->type == 1){
            glmDraw(&models[SPERM_MOD], GLM_COLOR | GLM_FLAT);
        } else if (this->type == 2){
            glScalef(.5, .5, 0.1);
            glRotatef (100, 1.0, 0.0, 0.0);
            // glRotatef (180, 0.0, 1.0, 0.0);
            glRotatef (90, 0.0, 0.0, 1.0);
            glmDraw(&models[R2_MOD], GLM_COLOR | GLM_FLAT);
        }
        glPopMatrix();
    }

    void reset_position(){
        this->xPos = rand() % 8 - 4;
        this->yPos = 3;
        this->type =  rand() % 3;
    }

    bool collide_with(Player player){
        float x_right = this->xPos + 0.25;
        float x_left = this->xPos - 0.25;
        float y_top = this->yPos + 0.25;
        float y_down = this->yPos - 0.25;
        float player_x_right = player.getXPos() + 0.25;
        float player_x_left = player.getXPos() - 0.25;
        float player_y_top = player.getYPos() + 0.5;
        float player_y_down = player.getYPos() - 0.5;
        // std::cout << "object: " << x_left << ", " << x_right << ", " << y_down << ", " << y_top << "player: " << player_x_left << ", " << player_x_right << ", " << player_y_down << ", " << player_y_top << std::endl;
        bool col = ((y_top >= player_y_down && y_down <= player_y_top) && (x_right >= player_x_left && x_left <= player_x_right));
        if (col){
            // std::cout<<"hey"<<std::endl;
            gHigh = Mix_LoadWAV( "/Users/taniagarridosalido/Dropbox/ITESM-ITC Decimo Semestre/Graficas/CondomWars/CondomWarsXC/audio/high.wav");
            if( gHigh == NULL )
            {
                printf( "Failed to load gSaber sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
            } else {
                Mix_PlayChannel( 1, gHigh, 0 );
            }
            //PLAY SOUND
        }
        return col;
    }
    
    void move_down(){
        if (collide_with(player)){
            if (type == 0 || type == 2){
                player.setScore(player.getScore() + 1);
            } else if (type == 1){
                player.setLives(player.getLives() - 1);
            }
            reset_position();
        } else {
            if(!(this->yPos < -4)){
                this->yPos = this->yPos - 0.1;
            }else{
                reset_position();
            }
        }
    }
};
////////////////////////////////////////////////////////////////////////////////

//////////////////////////// GLOBAL VARIABLES //////////////////////////////////

bool paused = false, started = false, menu = true;
int tenthsOfASecond = 0;
Object enemies[10];
int notUsed[10];
std::string fullPath = __FILE__;
const int TEXTURE_COUNT = 3;
const int TOTAL_OBJECTS = 5;
static GLuint texName[TEXTURE_COUNT];

////////////////////////////////////////////////////////////////////////////////

void loadTexture(Image* image,int k)
{
    
    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //Filtros de ampliacion y redución con cálculo mas cercano no es tan bueno pero es rápido
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    //Filtros de ampliacion y redución con cálculo lineal es mejo pero son más calculos
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
}

void getParentPath()
{
    for (int i = (int)fullPath.length()-1; i>=0 && fullPath[i] != '/'; i--) {
        fullPath.erase(i,1);
    }
}

std::string format(int tenthsOfASecond){
    std::ostringstream buffer;
    int seconds = tenthsOfASecond / 10;
    int tenths = tenthsOfASecond % 10;
    int minutes = seconds / 60;
    seconds %= 60;
    buffer << minutes;
    buffer << ':';
    if (seconds < 10){
        buffer << '0';
    }
    buffer << seconds;
    buffer << ':';
    buffer << tenths;
    return buffer.str();
}

void initializeNotUsed(){
    for (int i = 0; i < 10; i++) {
        notUsed[i] = 0;
    }
}

void generateEnemies(){
    int random;
    
    for (int i = 0; i < TOTAL_OBJECTS; i++) {
        enemies[i] = Object(rand() % 3,  rand() % 8 - 4);
    }
}

void displayEnemies(){
    for (int i = 0; i < TOTAL_OBJECTS; i++) {
        if (!(notUsed[i] == 0)){
            enemies[i].display();
            enemies[i].move_down();
        }
    }
}

void paintObjects(){
    if (tenthsOfASecond % TOTAL_OBJECTS == 0) {
        for (int i = 0; i < TOTAL_OBJECTS; i++) {
            if (notUsed[i] == 0){
                notUsed[i] = 1;
                break;
            }
        }
    }
}

void checkForGameOver(){
    if (player.getLives() <= 0){
        gameover = true;
        started = false;
        leia =  false;
        menu = false;
        paused = false;
    }
}

// Function for the timer
void timePassBy(int value){
    glutPostRedisplay();
    if (started) {
        tenthsOfASecond++;
    }
    if (!paused){
        glutTimerFunc(100, timePassBy, 0);
    }
    paintObjects();
}

// Function to display timer
void displayTime(){
    std::string time_formatted = format(tenthsOfASecond);
    glColor3f(1, 1, 1);
    glRasterPos2f(-4, 4);
    for (int i = 0; i < time_formatted.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, time_formatted[i]);
    }
}

// Function to generate player with defaults
void generatePlayer(){
    player = Player(0, "Luke", 1);
}

void displayPlayer(){
    player.display();
}

void displayScore(){
    std::ostringstream buffer;
    buffer << "Score: ";
    buffer << player.getScore();
    std::string score = buffer.str();
    glColor3f(1, 1, 1);
    glRasterPos2f(2, 4);
    for (int i = 0; i < score.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score[i]);
    }
}

void displayGameOverScore(){
    std::ostringstream buffer;
    buffer << "Final Score: ";
    buffer << player.getScore();
    std::string score = buffer.str();
    glColor3f(1, 1, 1);
    glRasterPos2f(1, -1);
    for (int i = 0; i < score.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score[i]);
    }
}

void displayLives(){
    std::ostringstream buffer;
    buffer << "Lives: ";
    buffer << player.getLives();
    std::string lives = buffer.str();
    glColor3f(1, 1, 1);
    glRasterPos2f(-1, 4);
    for (int i = 0; i < lives.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, lives[i]);
    }
}

void displayMenu(){
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, texName[0]);
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(4.5f, 4.5f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-4.5f, 4.5f, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void displayGameOver(){
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, texName[2]);
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(4.5f, 4.5f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-4.5f, 4.5f, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glutPostRedisplay();
}

void displayBackground(){
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, texName[1]);
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(4.5f, -4.5f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(4.5f, 4.5f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-4.5f, 4.5f, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void cargarmusica(){
    //Load sound effects
    gScratch = Mix_LoadMUS( "/Users/taniagarridosalido/Dropbox/ITESM-ITC Decimo Semestre/Graficas/CondomWars/CondomWarsXC/audio/imperial_march.wav" );
    if( gScratch == NULL )
    {
        printf( "Failed to load gSaber sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
    } else {
        if( Mix_PlayingMusic() == 0 )
        {
            //Play the music
            if( Mix_PlayMusic( gScratch, -1 ) == -1 )
            {
            }
        }
    }
}

void display(){
    cargarmusica();
    checkForGameOver();
    glPushMatrix();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    if (menu){
        glColor3f(1.0, 1.0, 1.0);
        displayMenu();
    } else if (started){
        displayBackground();
        displayTime();
        displayScore();
        displayLives();
        displayPlayer();
        displayEnemies();
    }
    if (gameover){
        glColor3f(1.0, 1.0, 1.0);
        displayGameOver();
        displayGameOverScore();
    }
    glutSwapBuffers();
    glPopMatrix();
}

void specialActions(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_RIGHT:
            if (started && !paused){
                player.move_right();
            }
            break;
        case GLUT_KEY_LEFT:
            if (started && !paused){
                player.move_left();
            }
            break;
    }
}

void keyboardActions(unsigned char theKey, int mouseX, int mouseY){
    switch (theKey){
        case 'r':
        case 'R':
            paused = true;
            started = false;
            tenthsOfASecond = 0;
            player.reset();
            break;
        case 'p':
        case 'P':
            if (started && !gameover){
                if (paused){
                    glutTimerFunc(100, timePassBy, 0);
                }
                paused = !paused;
            }
            break;
        case 'h':
        case 'H':
            if (!started){
                leia =  false;
                gameover = false;
                started = true;
                menu = false;
                paused = false;
                glutTimerFunc(100, timePassBy, 0);
            }
            break;
        case 'l':
        case 'L':
            if (!started){
                leia = true;
                started = true;
                menu = false;
                gameover = false;
                paused = false;
                glutTimerFunc(100, timePassBy, 0);
            }
            break;
        case 27:
            exit(0);
            break;
        default:
            break;// do nothing
    }
    glutPostRedisplay();
}

void reshape(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glOrtho(-250.0, 250.0, -250.0, 250.0, -250.0, 250.0);
    // glFrustum(-250.0, 250.0, -250.0, 250.0, -250.0, 250.0);
    glFrustum(-4.0, 4.0, -4.0, 4.0, 1.0, 4.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // gluLookAt(0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    // gluLookAt (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    gluLookAt(0, 0, 1.1, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void init(){
    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }

    
    //Habilitar el uso de texturas
    glClearColor (0, 0, 0, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    generatePlayer();
    generateEnemies();
}

void initRendering(){
    //Declaración del objeto Image
    Image* image;
    GLuint i=0;
    glEnable(GL_DEPTH_TEST);
    glGenTextures(TEXTURE_COUNT, texName); //Make room for our texture
    char ruta[200];
    sprintf(ruta,"%s%s", fullPath.c_str() , "images/start.bmp");
    image = loadBMP(ruta);
    loadTexture(image,i++);
    sprintf(ruta,"%s%s", fullPath.c_str() , "images/fondo_star_wars.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    sprintf(ruta,"%s%s", fullPath.c_str() , "images/gameover.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    delete image;
    std::string ruta_modelos = fullPath + "objects/leia/leia.obj";
    std::cout << "Filepath: " << ruta_modelos << std::endl;
    models[LEIA_MOD] = *glmReadOBJ(ruta_modelos.c_str());
    glmUnitize(&models[LEIA_MOD]);
    glmVertexNormals(&models[LEIA_MOD], 90.0, GL_TRUE);
    
    ruta_modelos = fullPath + "objects/han/han.obj";
    std::cout << "Filepath: " << ruta_modelos << std::endl;
    models[HAN_MOD] = *glmReadOBJ(ruta_modelos.c_str());
    glmUnitize(&models[HAN_MOD]);
    glmVertexNormals(&models[HAN_MOD], 90.0, GL_TRUE);

    ruta_modelos = fullPath + "objects/condom/condom.obj";
    std::cout << "Filepath: " << ruta_modelos << std::endl;
    models[CONDOM_MOD] = *glmReadOBJ(ruta_modelos.c_str());
    glmUnitize(&models[CONDOM_MOD]);
    glmVertexNormals(&models[CONDOM_MOD], 90.0, GL_TRUE);
    
    ruta_modelos = fullPath + "objects/sperm/sperm.obj";
    std::cout << "Filepath: " << ruta_modelos << std::endl;
    models[SPERM_MOD] = *glmReadOBJ(ruta_modelos.c_str());
    glmUnitize(&models[SPERM_MOD]);
    glmVertexNormals(&models[SPERM_MOD], 90.0, GL_TRUE);
    
    ruta_modelos = fullPath + "objects/r2-d2-obj/r2_d2.obj";
    std::cout << "Filepath: " << ruta_modelos << std::endl;
    models[R2_MOD] = *glmReadOBJ(ruta_modelos.c_str());
    glmUnitize(&models[R2_MOD]);
    glmVertexNormals(&models[R2_MOD], 90.0, GL_TRUE);
}

void timer(int value)
{
    glutPostRedisplay();
    // glutTimerFunc(1000,timer,0);
}

int main(int argc, char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(0,0);
    getParentPath();
    glutCreateWindow("Condom Wars");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardActions);
    glutSpecialFunc(specialActions);
    init();
    initRendering();
    glutTimerFunc(1000,timer,0);
    glutMainLoop();
}