#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/Picopixel.h>

// From original example "pcdtest"
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Working with my display:
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 6);

#define BOARD_MARGIN 2

#define BOARD_ROWS 10
#define BOARD_COLS 20

int head_row = 4;
int head_col = 0;

char board[BOARD_ROWS][BOARD_COLS] = {
  { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '^', 'v', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '^', 'v', '>', 'v', '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '^', '<', '^', 'v', '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '>', '>', '^', '>', '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  { '>', '>', '^', '>', '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }
};

void is_connected_with(const int &row, const int &col) {
  
}

void draw_snake() {
  int pos_y = 7;
  for (int row = 0; row < BOARD_ROWS; ++row) {
    int pos_x = 2;
    for (int col = 0; col < BOARD_COLS; ++col) {
        if (board[row][col] != ' ') {
          display.fillRect(
            pos_x,
            pos_y,
            3,
            3,
            BLACK
          );
        }
        pos_x += 4;
    }
    pos_y += 4;
  }
}

void setup() {
  display.begin();
  display.clearDisplay();
  display.setContrast(50);
  display.drawRect(0, 5, display.width()-1, display.height()-5, BLACK);

  display.setFont(&Picopixel);

  display.setCursor(0, 4);
  display.print("00:00");
  display.setCursor(display.width()-20, 4);
  char score[5];
  sprintf(score, "%05d", 123);
  display.print(score);

  draw_snake();

  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
