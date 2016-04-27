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
 * (I used a CD74HC4094E of Texas Instruments, but any SIPO with 8 parallel
 * outputs can be used).
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

#include "dcf77_receiver.h"
//http://www.arduino.cc/playground/Code/Time
#include <Time.h>
// http://playground.arduino.cc/Code/FrequencyTimer2
#include <FrequencyTimer2.h>

#define COLUMN_COUNT 9
#define ROW_COUNT 8

// The interval in which rows are changed for the LEDs, in microseconds
#define ROW_INTERVAL 2000

// The interval in which the DCF module should be checked for the current time, in milliseconds
#define TIME_CHECK_INTERVAL 1000

// The current hour and minute are displayed sequentially
#define STATE_RECEIVING_TIME 0
#define STATE_DISPLAYING_MINUTE 1
#define STATE_DISPLAYING_HOUR 2

int currentRow = 0;
int state = STATE_RECEIVING_TIME;

// Pins for controlling the 4094E shift register.
int strobePin = A2;
int dataPin = A0;
int clockPin = A1;

// Pins for reading the DCF 77 device
int dcfPin = 2; // Connection pin to DCF 77 device
int dcfInterruptPin = 0; // Interrupt number associated with pin
DCF77 dcf = DCF77(dcfPin, dcfInterruptPin);

// Pins connected to the columns
int columns[COLUMN_COUNT] = {A3, 4, 5, 6, 7, 8, 9, 10, 12};

void setup() {
        Serial.begin(9600);

        Serial.println("Initalizing PINS");

        // Initialize column pins as output
        for(int c = 0; c < COLUMN_COUNT; c++) {
                pinMode(columns[c], OUTPUT);
        }

        // Initialize shift register pins
        pinMode(strobePin, OUTPUT);
        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        digitalWrite(strobePin, LOW);
        digitalWrite(clockPin, LOW);

        Serial.println("Enabling timer");

        // Turn off toggling of pin 11
        FrequencyTimer2::disable();
        // Set refresh rate (interrupt timeout period) in microseconds
        FrequencyTimer2::setPeriod(ROW_INTERVAL);
        // Set interrupt routine to be called
        FrequencyTimer2::setOnOverflow(update);

        Serial.println("Starting DCF77 receiver");

        dcf.Start();
}

void loop() {
        time_t lastUpdate = dcf.getTime();
        if(lastUpdate != 0) {
                setTime(lastUpdate);
                Serial.println("Time received: ");
                digitalClockDisplay(lastUpdate);
                state = STATE_DISPLAYING_HOUR;
        }

        delay(TIME_CHECK_INTERVAL);
}

void digitalClockDisplay(time_t _time){
        tmElements_t tm;
        breakTime(_time, tm);

        Serial.print("Time: ");
        printDigits(tm.Hour);
        Serial.print(":");
        printDigits(tm.Minute);
        Serial.print(":");
        printDigits(tm.Second);
        Serial.print(" Date: ");
        Serial.print(tm.Day);
        Serial.print("-");
        Serial.print(tm.Month);
        Serial.print("-");
        Serial.println(tm.Year+1970);
}

void printDigits(int digits){
        if(digits < 10) {
                Serial.print('0');
        }
        Serial.print(digits);
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

void displayMinute(int minute){
        int row = minute / COLUMN_COUNT;
        int column = minute % COLUMN_COUNT;

        digitalWrite(columns[column], LOW);
        enableRow(row);
}

void displayHour(int hour){
        int row = ((hour%12)+60) / COLUMN_COUNT;
        int column = ((hour%12)+60) % COLUMN_COUNT;

        digitalWrite(columns[column], LOW);
        enableRow(row);
}

void displayTillMinute(int value){
        int row = value / COLUMN_COUNT;
        int c = -1;
        if(currentRow < row) {
                c = COLUMN_COUNT;
        }else if(currentRow == row) {
                c = value % COLUMN_COUNT;
        }

        for(int i = 0; i <= c; i++) {
                digitalWrite(columns[i], LOW);
        }
        enableRow(currentRow);
}


void clearColumns(){
        for (int c = 0; c < COLUMN_COUNT; c++) {
                digitalWrite(columns[c], HIGH);
        }
}

void animateReceiveProgress(){
        displayTillMinute(dcf.getBufferPosition());
        //displayTillMinute(counter);
}

void animateTime(){
        if(state == STATE_DISPLAYING_HOUR) {
                displayHour(hour());
        }else{
                displayMinute(minute());
        }

        state = (state == STATE_DISPLAYING_HOUR) ? STATE_DISPLAYING_MINUTE : STATE_DISPLAYING_HOUR;
}

// Interrupt routine
void update() {
        disableRows();
        clearColumns();
        if(state == STATE_RECEIVING_TIME) {
                currentRow += 1;
                currentRow %= ROW_COUNT;
                animateReceiveProgress();
        }else{
                animateTime();
        }



        /*row += 1;
           row %= ROW_COUNT;
           for (int column = 0; column < COLUMN_COUNT; column++) {
           if(column + COLUMN_COUNT * row == count){
            digitalWrite(columns[column], LOW);
           }else{
            digitalWrite(columns[column], HIGH);
           }
           }
           enableRow(row);*/
}
