/*
  LED CLOCK
 */

/* THIS STEALS PIN 3 AND 11 (http://playground.arduino.cc/Code/FrequencyTimer2) */

/*
 *    MATRIX LAYOUT
 *
 *   | 0   1   2   3   4   5   6   7   8
 * --+-------------------------------------
 * 0 | 0   1   2   3   4   5   6   7   8
 * 1 | 9   10  11  12  13  14  15  16  17
 * 2 | 18  19  20  21  22  23  24  25  26
 * 3 | 27  28  29  30  31  32  33  34  35
 * 4 | 36  37  38  39  40  41  42  43  44
 * 5 | 45  46  47  48  49  50  51  52  53
 * 6 | 54  55  56  57  58  59  h0  h1  h2
 * 7 | h3  h4  h5  h6  h7  h8  h9  h10 h11
 */

#include <FrequencyTimer2.h>

#define COLUMN_COUNT 9
#define ROW_COUNT 1
#define COLUMN_INTERVAL 2000

byte column = 0;

int hour = 0;
int minute = 0;

int columns[COLUMN_COUNT] = {2, 4, 5, 6, 7, 8, 9, 10, 12};
int rows[ROW_COUNT] = {10};

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  for(int c = 0; c < COLUMN_COUNT; c++){
    pinMode(columns[c], OUTPUT);
  }
  for(int r = 0; r < ROW_COUNT; r++){
    pinMode(rows[r], OUTPUT);
  }

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);

  // Turn off toggling of pin 11
  FrequencyTimer2::disable();
  // Set refresh rate (interrupt timeout period) in microseconds
  FrequencyTimer2::setPeriod(COLUMN_INTERVAL);
  // Set interrupt routine to be called
  FrequencyTimer2::setOnOverflow(display);
}

// the loop function runs over and over again forever
void loop() {
  //digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);              // wait for a second
  //digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  //delay(1000);              // wait for a second
}

// Interrupt routine
void display() {
  digitalWrite(columns[column], HIGH);  // Turn whole previous column off
  column++;
  if (column == COLUMN_COUNT) {
    column = 0;
  }
  for (int row = 0; row < ROW_COUNT; row++) {
    digitalWrite(rows[row], HIGH);
  }
  digitalWrite(columns[column], LOW); // Turn whole column on at once (for equal lighting times)
}
