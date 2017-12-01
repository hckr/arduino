#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/Picopixel.h>

// From original example "pcdtest"
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// My configuration
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 4, 5, 9, 6);

#define BOARD_MARGIN_LEFT 2
#define BOARD_MARGIN_TOP 7

#define BOARD_ROWS 10
#define BOARD_COLS 20

#define SNAKE_FRAGMENT_SIZE 3
// only from right and bottom:
#define SNAKE_CONNECTION_MARGIN 1

#define PIN_LEFT 7
#define PIN_UP 1
#define PIN_RIGHT 2
#define PIN_DOWN 3

int head_row = 4;
int head_col = 4;

int tail_row = head_row;
int tail_col = 0; // see setup()

enum Direction {
  LEFT,
  UP,
  RIGHT,
  DOWN
};

Direction snake_direction = RIGHT;
volatile Direction proposed_direction = RIGHT;

unsigned long last_update = 0;
unsigned int score = 0;
bool game_over = false;

enum BoardFieldType {
  EMPTY,
  SNAKE,
  FOOD,
  DEATH
};

struct BoardField {
  BoardFieldType type = EMPTY;
  // used only by SNAKE type:
  int8_t next_row = -1;
  int8_t next_col = -1;
};

struct BoardField board[BOARD_ROWS][BOARD_COLS];
struct BoardField out_of_board; // initialized in setup()

struct BoardField& get_board_field(const int &row, const int &col) {
  if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) {
    return out_of_board;
  }
  return board[row][col];
}

void draw_snake_and_food() {
  int pos_y = BOARD_MARGIN_TOP;
  for (int row = 0; row < BOARD_ROWS; ++row) {
    int pos_x = BOARD_MARGIN_LEFT;
    for (int col = 0; col < BOARD_COLS; ++col) {
        switch (get_board_field(row, col).type) {
          case SNAKE:
            display.fillRect(
              pos_x,
              pos_y,
              SNAKE_FRAGMENT_SIZE,
              SNAKE_FRAGMENT_SIZE,
              BLACK
            );
            break;
          case FOOD:
            display.drawPixel(
              pos_x + 1,
              pos_y,
              BLACK
            );
            display.drawPixel(
              pos_x,
              pos_y + 1,
              BLACK
            );
            display.drawPixel(
              pos_x + 2,
              pos_y + 1,
              BLACK
            );
            display.drawPixel(
              pos_x + 1,
              pos_y + 2,
              BLACK
            );
            break;
        }
        pos_x += SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN;
    }
    pos_y += SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN;
  }
}

void draw_connections() {
  int curr_row = tail_row,
      curr_col = tail_col;
  while (1) {
    const auto &curr = get_board_field(curr_row, curr_col);
    const auto &next = get_board_field(curr.next_row, curr.next_col);
    if (curr.type != SNAKE || next.type != SNAKE) {
      break;
    }
    const auto row_diff = curr.next_row - curr_row;
    const auto col_diff = curr.next_col - curr_col;
    if (row_diff == 1) { // from the bottom
      display.fillRect(
        BOARD_MARGIN_LEFT + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr_col,
        BOARD_MARGIN_TOP + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr_row + SNAKE_FRAGMENT_SIZE,
        SNAKE_FRAGMENT_SIZE,
        SNAKE_CONNECTION_MARGIN,
        BLACK
      );
    } else if (row_diff == -1) { // from the top - add to the next
      display.fillRect(
        BOARD_MARGIN_LEFT + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr.next_col,
        BOARD_MARGIN_TOP + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr.next_row + SNAKE_FRAGMENT_SIZE,
        SNAKE_FRAGMENT_SIZE,
        SNAKE_CONNECTION_MARGIN,
        BLACK
      );
    } else if (col_diff == 1) { // from the right
      display.fillRect(
        BOARD_MARGIN_LEFT + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr_col + SNAKE_FRAGMENT_SIZE,
        BOARD_MARGIN_TOP + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr_row,
        SNAKE_CONNECTION_MARGIN,
        SNAKE_FRAGMENT_SIZE,
        BLACK
      );
    } else if (col_diff == -1) { // from the left - add to the next
      display.fillRect(
        BOARD_MARGIN_LEFT + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr.next_col + SNAKE_FRAGMENT_SIZE,
        BOARD_MARGIN_TOP + (SNAKE_FRAGMENT_SIZE + SNAKE_CONNECTION_MARGIN) * curr.next_row,
        SNAKE_CONNECTION_MARGIN,
        SNAKE_FRAGMENT_SIZE,
        BLACK
      );
    }
    curr_row = curr.next_row;
    curr_col = curr.next_col;
  }
}

void place_food() {
  do {
    int food_row = random(0, BOARD_ROWS),
        food_col = random(0, BOARD_COLS);

    auto &food = get_board_field(food_row, food_col);
    if (food.type == EMPTY) {
      food.type = FOOD;
      break;
    }
  } while (1);
}

void step() {
  int new_row = head_row,
      new_col = head_col;
  
  switch (snake_direction) {
    case LEFT:
      new_col -= 1;
      break;
    case UP:
      new_row -= 1;
      break;
    case RIGHT:
      new_col += 1;
      break;
    case DOWN:
      new_row += 1;
      break;
  }

  bool grow = false;

  struct BoardField& new_head = get_board_field(new_row, new_col);
  switch (new_head.type) {
    case SNAKE:
    case DEATH:
      game_over = true;
      return;
    case FOOD:
      score += 1;
      grow = true;
      // no break
    case EMPTY:
      new_head.type = SNAKE;
      new_head.next_row = -1; // IMPORTANT – solved random crashes
      new_head.next_col = -1; //
      struct BoardField& old_head = get_board_field(head_row, head_col);
      old_head.next_row = new_row;
      old_head.next_col = new_col;
      head_row = new_row;
      head_col = new_col;
  }

  if (grow) {
    place_food();
  } else {
    struct BoardField& old_tail = get_board_field(tail_row, tail_col);
    tail_row = old_tail.next_row;
    tail_col = old_tail.next_col;
    old_tail.type = EMPTY;
  }
}

void draw() {
  display.clearDisplay();
  display.setCursor(0, 4);
  char timer_str[5];
  long full_seconds = millis() / 1000,
       minutes = full_seconds / 60,
       seconds = full_seconds % 60;
  sprintf(timer_str, "%02ld:%02ld", minutes, seconds);
  display.print(timer_str);
  display.setCursor(display.width()-20, 4);
  char score_str[5];
  sprintf(score_str, "%05u", score);
  display.print(score_str);
  display.drawRect(0, 5, display.width()-1, display.height()-5, BLACK);
  draw_snake_and_food();
  draw_connections();
  display.display();
}

void move_left() {
  if (snake_direction != RIGHT) {
    proposed_direction = LEFT;
  }
}

void move_up() {
  if (snake_direction != DOWN) {
    proposed_direction = UP;
  }
}

void move_right() {
  if (snake_direction != LEFT) {
    proposed_direction = RIGHT;
  }
}

void move_down() {
  if (snake_direction != UP) {
    proposed_direction = DOWN;
  }
}

void setup() {
  randomSeed(analogRead(A0));
  
  out_of_board.type = DEATH;
  
  get_board_field(head_row, head_col).type = SNAKE;
  
  for (int c = head_col - 1; c >= 0; --c) {
    auto &f = get_board_field(head_row, c);
    f.type = SNAKE;
    f.next_row = head_row;
    f.next_col = c + 1;
  }

  pinMode(PIN_LEFT, INPUT);
  pinMode(PIN_UP, INPUT);
  pinMode(PIN_RIGHT, INPUT);
  pinMode(PIN_DOWN, INPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_LEFT), move_left, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_UP), move_up, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), move_right, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_DOWN), move_down, RISING);
  
  display.begin();
  display.clearDisplay();
  display.display();
  display.setContrast(50);
  display.setFont(&Picopixel);

  place_food();
  draw();
}

void loop() {
//  if (!game_over) {
    if (millis() - last_update >= 500) {
      last_update = millis();
      snake_direction = proposed_direction;
      step();
    }
//  }
  draw();
}

