#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/Picopixel.h>

// From original example "pcdtest"
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Working with my display:
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 6);

#define BOARD_MARGIN_LEFT 2
#define BOARD_MARGIN_TOP 7

#define BOARD_ROWS 10
#define BOARD_COLS 20

#define SNAKE_FRAGMENT_SIZE 3
// only from right and bottom:
#define SNAKE_CONNECTION_MARGIN 1

int head_row = 4;
int head_col = 0;

int tail_row = head_row;
int tail_col = head_col;

enum Direction {
  LEFT,
  UP,
  RIGHT,
  DOWN
} snake_direction = RIGHT;

int last_update = 0;
int score = 0;
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
  int next_row = -1;
  int next_col = -1;
};

struct BoardField board[BOARD_ROWS][BOARD_COLS];
struct BoardField out_of_board; // initialized in setup()

struct BoardField& get_board_field(const int &row, const int &col) {
  if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) {
    return out_of_board;
  }
  return board[row][col];
}

bool is_part_of_snake(const int &row, const int &col) {
  const auto &f = get_board_field(row, col);
  return f.type == SNAKE;
}

void draw_snake() {
  int pos_y = BOARD_MARGIN_TOP;
  for (int row = 0; row < BOARD_ROWS; ++row) {
    int pos_x = BOARD_MARGIN_LEFT;
    for (int col = 0; col < BOARD_COLS; ++col) {
        if (is_part_of_snake(row, col)) {
          display.fillRect(
            pos_x,
            pos_y,
            SNAKE_FRAGMENT_SIZE,
            SNAKE_FRAGMENT_SIZE,
            BLACK
          );
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
    if (curr.type != SNAKE) {
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
      struct BoardField& old_head = get_board_field(head_row, head_col);
      old_head.next_row = new_row;
      old_head.next_col = new_col;
      head_row = new_row;
      head_col = new_col;
  }

  if (!grow) {
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
  int full_seconds = millis() / 1000,
      minutes = full_seconds / 60,
      seconds = full_seconds % 60;
  sprintf(timer_str, "%02d:%02d", minutes, seconds);
  display.print(timer_str);
  display.setCursor(display.width()-20, 4);
  char score_str[5];
  sprintf(score_str, "%05d", score);
  display.print(score_str);
  display.drawRect(0, 5, display.width()-1, display.height()-5, BLACK);
  draw_snake();
  draw_connections();
  display.display();
}

void setup() {
  out_of_board.type = DEATH;
  
  display.begin();
  display.clearDisplay();
  display.display();
  display.setContrast(50);
  display.setFont(&Picopixel);

  draw();
}

void loop() {
  if (!game_over) {
    if (millis() - last_update >= 1000) {
      step();
      draw();
      last_update = millis();
    }
  }
}

