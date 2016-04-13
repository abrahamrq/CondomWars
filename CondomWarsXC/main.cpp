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

////////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <fstream>

using namespace std;

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
        lives = 3;
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
        glColor3f(0, 0, 0);
        glPushMatrix();
        glTranslated(this->xPos, this->yPos, 0);
        glScalef(.5, .5, 0.1);
        glutWireCube(1);
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
        glColor3f(0, 0, 0);
        glPushMatrix();
        glTranslated(this->xPos, this->yPos, 0);
        glScalef(.5, .5, 0.1);
        glutWireSphere(.5, 10, 10);
        glPopMatrix();
    }
    
    void move_down(){
        if(!(this->yPos < -3)){
            this->yPos = this->yPos - 0.1;
        }else{
            this->yPos = 3;
        }
    }
};
////////////////////////////////////////////////////////////////////////////////

//////////////////////////// GLOBAL VARIABLES //////////////////////////////////

bool paused = false, started = false, menu = true;
Player player;
int tenthsOfASecond = 0;
Object enemies[10];
int notUsed[10];
std::string fullPath = __FILE__;
const int TEXTURE_COUNT=2;
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
    
    for (int i = 0; i < 10; i++) {
        random = rand() % 8 - 4;
        enemies[i] = Object(1, random);
    }
}

void displayEnemies(){
    for (int i = 0; i < 10; i++) {
        if (!(notUsed[i] == 0)){
            enemies[i].display();
            enemies[i].move_down();
        }
    }
}

void paintObjects(){
    if (tenthsOfASecond % 10 == 0) {
        for (int i = 0; i < 10; i++) {
            if (notUsed[i] == 0){
                notUsed[i] = 1;
                break;
            }
        }
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
    glColor3f(0, 0, 0);
    glRasterPos2f(-4, 4);
    for (int i = 0; i < time_formatted.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, time_formatted[i]);
    }
}

// Function to generate player with defaults
void generatePlayer(){
    player = Player(0, "Luke", 3);
}

void displayPlayer(){
    player.display();
}

void displayScore(){
    std::ostringstream buffer;
    buffer << "Score: ";
    buffer << player.getScore();
    std::string score = buffer.str();
    glColor3f(0, 0, 0);
    glRasterPos2f(2, 4);
    for (int i = 0; i < score.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score[i]);
    }
}

void displayLives(){
    std::ostringstream buffer;
    buffer << "Lives: ";
    buffer << player.getLives();
    std::string lives = buffer.str();
    glColor3f(0, 0, 0);
    glRasterPos2f(-1, 4);
    for (int i = 0; i < lives.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, lives[i]);
    }
}

void displayMenu(){
    glBindTexture(GL_TEXTURE_2D, texName[0]);
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-5.0f, -5.0f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(5.0f, -5.0f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(5.0f, 5.0f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-5.0f, 5.0f, 0);
    glEnd();
}

void displayBackground(){
    glBindTexture(GL_TEXTURE_2D, texName[1]);
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-5.0f, -5.0f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(5.0f, -5.0f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(5.0f, 5.0f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-5.0f, 5.0f, 0);
    glEnd();
}

void display(){
    glPushMatrix();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    if (menu){
        displayMenu();
    } else {
        displayBackground();
        displayTime();
        displayScore();
        displayLives();
        displayPlayer();
        displayEnemies();
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
    glutPostRedisplay();
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
            if (started){
                if (paused){
                    glutTimerFunc(100, timePassBy, 0);
                }
                paused = !paused;
            }
            break;
        case 's':
        case 'S':
            if (!started){
                started = true;
                menu = false;
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
    //Habilitar el uso de texturas
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glClearColor (0.5, 0.5, 0.5, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    generatePlayer();
    generateEnemies();
}

void initRendering(){
    //Declaración del objeto Image
    Image* image;
    GLuint i=0;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(TEXTURE_COUNT, texName); //Make room for our texture
    char ruta[200];
    sprintf(ruta,"%s%s", fullPath.c_str() , "images/menu1.bmp");
    image = loadBMP(ruta);
    loadTexture(image,i++);
    sprintf(ruta,"%s%s", fullPath.c_str() , "images/fondo.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    delete image;
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