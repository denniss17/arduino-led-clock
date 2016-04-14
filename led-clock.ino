/*
 * LED clock
 *
 * This is a driver for an Arduino (Nano v3) for controlling a clock made out
 * of 72 LEDs, 60 of which are in a circle for indicating the current minute
 * and another 12 for indicating the current hour.
 *
 * The LEDs are connected as if it were a LED matrix.
 * There are 8 rows and 9 columns, and the layout of the LEDs in this matrix
 * is as displayed in the scheme below.
 *
 * The columns are directly attached to pins of the Arduino, while the rows
 * are controlled via a SIPO (serial in parallel out) shift register
 * (I used a CD74HC4094E of Texas Instruments).
 *
 * Required libraries:
 * - FrequenyTimer 2
 *   (http://playground.arduino.cc/Code/FrequencyTimer2)
 *   This steals pin 3 and 11
 */

/*
 * Relevant links
 *
 * http://www.ti.com/lit/ds/symlink/cd54hc4094.pdf
 * http://playground.arduino.cc/Code/FrequencyTimer2
 */

/*
 * Matrix layout (h = hour)
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
#define ROW_COUNT 8
#define ROW_INTERVAL 2000

byte row = 0;

int hour = 0;
int minute = 0;

int count = 0;

// Pins for controlling the 4094E shift register.
int strobePin = A2;
int dataPin = A0;
int clockPin = A1;

int columns[COLUMN_COUNT] = {2, 4, 5, 6, 7, 8, 9, 10, 12};

// the setup function runs once when you press reset or power the board
void setup() {
  // Initialize column pins as output
  for(int c = 0; c < COLUMN_COUNT; c++){
    pinMode(columns[c], OUTPUT);
  }

  // Initialize shift register pins
  pinMode(strobePin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(strobePin, LOW);
  digitalWrite(clockPin, LOW);

  // Turn off toggling of pin 11
  FrequencyTimer2::disable();
  // Set refresh rate (interrupt timeout period) in microseconds
  FrequencyTimer2::setPeriod(ROW_INTERVAL);
  // Set interrupt routine to be called
  FrequencyTimer2::setOnOverflow(update);
}

void loop() {
  delay(50);

  count += 1;
  count %= 60;
}

void disableRows(){
  // Set the strobe pin to low, so the change will not have immediate effect
  digitalWrite(strobePin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, 0);
  // Set the strobe pin high to transfer bits to the storage
  digitalWrite(strobePin, HIGH);
}

void enableRow(int row){
  // Set the strobe pin to low, so the change will not have immediate effect
  digitalWrite(strobePin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, 0x01 << row);
  // Set the strobe pin high to transfer bits to the storage
  digitalWrite(strobePin, HIGH);
}

// Interrupt routine
void update() {
  disableRows();
  row += 1;
  row %= ROW_COUNT;
  for (int column = 0; column < COLUMN_COUNT; column++) {
    if(column + COLUMN_COUNT * row == count){
      digitalWrite(columns[column], LOW);
    }else{
      digitalWrite(columns[column], HIGH);
    }
  }
  enableRow(row);
}
