/*
 * Author: Diego Avendano
 * 
 * The screen library and the first sketch is taken from Adafruit tutorial for the screen.
 * 
 * To control the snake, use the COM port (TODO: Add buttons). In github, with this file I added a simple nodejs server to control it through the browser with the keyboard or the screen.
 * 
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD reset (RST)
// pin 3 - LCD chip select (CS)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

#define XPOS 0
#define YPOS 1

#define UP -1
#define DOWN 1
#define LEFT -1
#define RIGHT 1

#define LIMITX 83
#define LIMITY 47

#define LIMSNAKEX 26        // 80/3 points (each point = 3px)
#define LIMSNAKEY 14        // 44/3 points (each point = 3px)

#define STARTSNAKEX 4    // Begining in 1, same as init snake length
#define STARTSNAKEY 7

#define INCREMENT 2
#define INIT_LEVEL 5

#define SNAKEMAX 364

// Type definitions
typedef struct snk_coord {
  int8_t x;
  int8_t y;
} snk_coord;

snk_coord snk_body[SNAKEMAX];

typedef struct snk_snake {
  snk_coord pos;
  snk_coord dir;
  uint16_t len;
  snk_coord* body;
} snk_snake;

typedef struct snk_map{
  snk_snake snake;
  snk_coord food;
  uint8_t level;
} snk_map;

// Global variables
bool alive;
bool eaten;
bool pause;

// Use when printing the pressed 
// int button;

// Setup
void setup() {
  Serial.begin(9600);

  display.begin();

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);

  // Uncomment to show Splashscreen
  //display.display(); // show splashscreen
  //delay(500);
  display.clearDisplay();   // clears the screen and buffer

  display.fillScreen(BLACK);
  drawText ("Snake", 5, 2, 12, 16, WHITE, BLACK);
  display.display();
  delay(1000);
  display.clearDisplay();  
  
//  display.drawRect(0, 0, LIMITX, LIMITY, BLACK);
//  drawText ("Loading...", 10, 1, 14, 19, BLACK, WHITE);
//  display.display();
//  delay(0);  
}

// Loop
void loop() {  
  snk_map  map = init_snake(); 
  drawMap(map);
  
  alive = true;
  
  while (alive) {
    
    // Check input
    do {
      checkInput (&map.snake);
    } while (pause);
        
    // Move snake
    moveSnake (&map.snake);

    // Checks
    alive = !(checkLimits(map.snake) | checkInSnake(map.snake, map.snake.pos));
    eaten = checkFood(map);
    if (eaten) {
      map.food = createFood(map);
      map.snake.len+=INCREMENT;
      (map.level>14)?:map.level++;
    }
    
    // Redraw    
    drawMap(map);
    
    delay (800-map.level*50);
  }
}

/*
 * Draws a 3x3 point
 * The coordinates are in points, not in px 
 * If isSnake is true, the point is full, if false, only diagonals
 *                                        o  o  o                                     o     o
 *                                        o  o  o                                         o 
 *                                        o  o  o                                     o     o
 */
void drawPoint (uint8_t x_, uint8_t y_, int color, bool isSnake) {
  uint8_t x = 3*x_-1;
  uint8_t y = 3*y_-1;
  display.drawPixel(x, y, color);
  display.drawPixel(x-1, y+1, color);
  display.drawPixel(x+1, y+1, color);
  display.drawPixel(x-1, y-1, color);
  display.drawPixel(x+1, y-1, color);

  if (isSnake) {
    display.drawPixel(x, y+1, color);
    display.drawPixel(x-1, y, color);
    display.drawPixel(x+1, y, color);
    display.drawPixel(x, y-1, color);
  }
}

/*
 * Prints text (array of char)
 * sSize is the size of the array of chars and fSize is the font size.
 * bg is the background color. Only in the text part, if negative wanted, use with display.fillScreen(BLACK)
 */
void drawText (char* text, uint16_t sSize, uint8_t fSize, uint8_t x, uint8_t y, uint8_t color, uint8_t bg) {
  for (uint16_t i = 0; i < sSize; i++) {
    display.drawChar(x+i*fSize*6, y, text[i], color, bg, fSize);
  }
}

/* 
 *  Initialization of the game
 *  Initializes the food, the snake and the map.
 *  returns the struct of the map, which also contains the food and snake structs
 */
struct snk_map init_snake() {
  // Food
  uint8_t food_x = 15;
  uint8_t food_y = STARTSNAKEY;
  snk_coord food = {food_x, food_y};

  // Snake
  for (int16_t i = 0; i < STARTSNAKEX; i++) {
    snk_body[i].x = STARTSNAKEX - i-1;
    snk_body[i].y = STARTSNAKEY;
  }
    
  for (int16_t i = STARTSNAKEX; i < SNAKEMAX; i++) {
    snk_body[i].x = -1;
    snk_body[i].y = -1;    
  }
 
  snk_coord pos = {STARTSNAKEX-1, STARTSNAKEY};
  snk_coord dir = {1, 0};

  snk_snake snake = {
    pos,
    dir,
    STARTSNAKEX,
    snk_body,
  };

  snk_map map = {
    snake,
    food,
    INIT_LEVEL,
  };

  return map;
}

/*
 * Redraws the fotogram on the screen
 * It needs the map to draw, which contains all the information about the snake position and the food
 */
void drawMap (snk_map map) {
  display.clearDisplay();

  if (!alive) {
    display.fillScreen(BLACK);
  } else {  
    display.drawRect(0, 0, LIMITX, LIMITY, BLACK);
  
    // Draw food
    drawPoint(map.food.x, map.food.y, BLACK, false);
  
    // Draw snake
    for (int16_t i = 0; i < map.snake.len; i++) {
        drawPoint(map.snake.body[i].x, map.snake.body[i].y, BLACK, true);
    }
  }
  display.display();
}

/*
 * Prints the position of the body elements of the snake to the Serial (Debug)
 */
void printSnake (snk_coord* body, uint16_t len) {
  for (int i = 0; i < len+4; i++) {
    Serial.print("x: ");
    Serial.print(body[i].x);
    Serial.print(", y: ");
    Serial.print(body[i].y);
    Serial.print("\n");
  }
}

/*
 * Prints the direction that the snake is following to the Serial (Debug)
 */
void printSnakeDir(snk_coord dir) {
    Serial.print("x: ");
    Serial.print(dir.x);
    Serial.print(", y: ");
    Serial.print(dir.y);
    Serial.print("\n");
}

/*
 * Checks if an input has been entered (via Serial)
 * If a valid command has been entered, executes the action (change of direction when applicable or pause/resume the game)
 * Eliminates the DLE character that some terminals add.
 */
void checkInput (snk_snake* snake) {
  int button = (int)Serial.read();
  if (button == 10)         // To get rid of the DLE character of the terminal
    button = (int)Serial.read();;
  button -= 32;

  snk_coord dir = snake->dir;
  switch (button) {
    case 87:
      if (dir.y == 0) {
        dir.x = 0;
        dir.y = UP;
      }
      break;
    case 65:
      if (dir.x == 0) {
        dir.x = LEFT;
        dir.y = 0;
      }
      break;
    case 83:
      if (dir.y == 0) {
        dir.x = 0;
        dir.y = DOWN;
      }
      break;
    case 68:
      if (dir.x == 0) {
        dir.x = RIGHT;
        dir.y = 0;
      }
      break;
    case 80:
       pause = !pause;
    default:
      break;
    //break;
  }

  if(!pause)
    snake->dir = dir;
  //button = -1;
}

/* 
 *  Updates the position of the snake and the elements of the body.
 */
void moveSnake (snk_snake* snake) {
  snake->pos.x += snake->dir.x;
  snake->pos.y += snake->dir.y;   
 
  for (int16_t i = snake->len-1; i >0; i--) {
    snake->body[i].x = snake->body[i-1].x;
    snake->body[i].y = snake->body[i-1].y;
  }
  
  snake->body[0].x = snake->pos.x;
  snake->body[0].y = snake->pos.y;   
}

/*
 * Checks if the position of the snake is out of the limits (true when out)
 */
bool checkLimits (snk_snake snake) {
  if (snake.pos.x > LIMSNAKEX+1 | snake.pos.y > LIMSNAKEY+1 | snake.pos.x < 1 | snake.pos.y < 1)
    return true;
  return false;
}

/*
 * Checks if a position belongs to the snake, to check if the snake hit itself or to check that the food is created out of the snake
 */
bool checkInSnake(snk_snake snake, snk_coord pos) {
  for (int16_t i = 1; i < snake.len; i++) {
    if (pos.x == snake.body[i].x && pos.y == snake.body[i].y)
      return true;
  }
  return false;
}

/*
 * Checks if the snake has found the food
 */
bool checkFood (snk_map map) {
  if (map.snake.pos.x == map.food.x && map.snake.pos.y == map.food.y)
    return true;
  return false;
}

/*
 * Creates the food position randomly (Within the limits and not in the snake)
 */
snk_coord createFood (snk_map map) {
  snk_coord food;
  do {
    food.x = random (1, LIMSNAKEX);
    food.y = random (1, LIMSNAKEY);
  } while (checkInSnake(map.snake, food));
  return food;
}

// To show pressed key and make button global
//  char buf[3];
//  sprintf (buf, "%d", button);
//  drawText (buf, 3, 1, 5, 5, BLACK, WHITE);

// TODO
// Position of snake using Linked Lists? (Better to update?, more memory?)
// Control with buttons and interruptions instead of Serial
// Improve createFood algorithm (list available coordinates and random from that)
