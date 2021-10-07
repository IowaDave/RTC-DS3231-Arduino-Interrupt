/* RTC-DS3231-Arduino
 * (c) 2021 by David Sparks
 * Freely and gladly released into the public domain.
 * Anyone may use it any way they like.
 * 
 * Function: to demonstrate using DS3231 Real Time Clock 
 * with Arduino Interrupts
 * 
 * Optionally, set the time on the DS3231 real-time clock (RTC).
 * 
 * Check the time at 10-second intervals 
 * using the RTC's alarm and an interrupt
 */

/**************************************************************
 * NOTE TO READERS:
 * I write lengthy comments because I will refer to them again.
 * In the future I will need my code to explain clearly to me
 * why it is doing what it is doing.
 **************************************************************/

#include <DS3231.h> // RTC library by Andrew Wickert
#include <Wire.h> // I2C library

#ifndef SECONDS_FROM_1970_TO_2000
#define SECONDS_FROM_1970_TO_2000 946684800
#endif

// create DS3231 object
DS3231 clock; // class defined in DS3231.h

// create DateTime object
DateTime dt; // datetime class defined in DS3231.h

// Arduino uses these with the interrupt alarm from DS3231
const int dataPin = 3; // for Uno.  Use pin 7 for Leonardo
volatile bool alarmEventFlag = false;

/* The following line defines the Interrupt Service Routine (ISR).
 * An ISR is the code that the CPU will execute immediately
 * when an interrupt occurs.
 * This one sets a flag telling the sketch that
 * the interrupt did occur. The sketch can test the flag
 * and take appropriate action when it is "true".
 * ISR's should be kept as short as possible. 
 * It is sufficient only to change the value of a single,
 * "global" variable, like this.
 */
void rtcISR() {alarmEventFlag = true;}

/* Here are prototypes for alarm-related "helper" functions,
 * that will be defined below, following the main loop.
 */
DateTime addSecondsToTime( int secondsToAdd, DateTime theTime);
void setTheAlarm(DateTime alarmTime);
void runTheSpecialCode();

/*
// Only have to set the time once, if using backup battery in DS3231.
// I established the starting time at
// Wednesday, October 6, 2021, 19:16:00 (7:19 p.m.)
byte year = 21; // last two digits of 2021
byte month = 10;
byte day = 6;
byte dow = 4; // I instituted Sunday = 1, thus 4 = Wednesday
byte hour = 19; // 7 pm
byte minute = 16;
byte second = 0; // why not
bool clockMode = false; // use 24-hour mode
*/

void setup() {
  // initiate communications
  Wire.begin(); // I2C channel for the DS3231
  Serial.begin(57600); // for the Arduino's Serial Monitor
  
  // The following code line is needed by 
  // Leonardo and other ATMega32u4-based boards.
  // It waits for the computer to activate the serial port.
  // Uno and other types of Arduinos are not affected by it.
  while (!Serial) {}

  // Initialize the data pin
  // to sense signals from the DS3231
  pinMode(dataPin, INPUT);

  /* tell Arduino to use the data pin for interrupts
   * and to call the function named rtcISR()
   * when a FALLING signal comes in from the DS3231.
   */
  attachInterrupt(digitalPinToInterrupt(dataPin), rtcISR, FALLING);
  
  Serial.println("Begin the test");

/*
// Again, only have to set the clock once. Afterward,
// the DS3231 can maintain the time with a battery. 
  // set the clock
  clock.setYear(year);
  clock.setMonth(month);
  clock.setDate(day);
  clock.setDoW(dow);
  clock.setHour(hour);
  clock.setMinute(minute);
  clock.setSecond(second);
  clock.setClockMode(clockMode);
*/
  // read the clock
  dt = RTClib::now(); // RTC class defined in DS3231.h

  // print the current date
  Serial.print(dt.month()); Serial.print("/");
  Serial.print(dt.day()); Serial.print("/");
  Serial.println(dt.year());

  // print the current time
  Serial.print(dt.hour()); Serial.print(":");
  if (dt.minute() < 10) Serial.print("0");
  Serial.print(dt.minute()); Serial.print(":");
  if (dt.second() < 10) Serial.print("0");
  Serial.println(dt.second());

  // add 10 seconds to the time
  dt = addSecondsToTime(10, dt);

  // print the new time
  Serial.print("First alarm set to time: ");
  Serial.print(dt.hour()); Serial.print(":");
  if (dt.minute() < 10) Serial.print("0");
  Serial.print(dt.minute()); Serial.print(":");
  if (dt.second() < 10) Serial.print("0");
  Serial.println(dt.second());

  // set the alarm to the new time
  setTheAlarm(dt);

  // begin reporting alarm events
  Serial.println ("Alarm events:");
  
}

void loop() {

  // The main loop needs only to check a flag
  // to see whether an alarm has come in.
  // It does not need to keep track of time,
  // because the DS3231 takes care of that.
  
  if (alarmEventFlag == true) {
    // run the special code
    runTheSpecialCode();
    // turn off this code's alarm flag
    alarmEventFlag = false;
  } // alarmEvent

  // The main loop can stay busy doing other things.
  // The DS3231 will interrupt when it signals an alarm.

}  // end of the main loop

// helper functions

/* addSecondsToTime() uses an "unmentioned" feature
 * of the DS3231 library. It turns out there
 * is a simple way to add any number of seconds
 * to a DateTime type of variable. You won't find it
 * listed among the available functions given on
 * the README page of the DS3231 library.
 * It's hinted at in the DS3231.h file and
 * defined in the DS3231.cpp file.
 * It pays to RFTC (Read The Fine Code)!
 */
DateTime addSecondsToTime(int secondsToAdd, DateTime theTime) {
  // the "unixtime()" function returns the DS3231 time
  // as number of seconds in an unisgned long integer.
  // NOTE TO FUSSY PEOPLE: this number might not equal  
  // what you would expect for the conventional "Unix Time".
  // It does not matter.
  // For this purpose, we briefly need only a number of seconds.
  uint32_t theTimeInSeconds = dt.unixtime(); 
  
  // It turns out that a new DataTime variable can be defined
  // by supplying a time as a number of seconds.
  // (Technically, the DateTime object constructor is overloaded,
  //  and this is one of the declarations for it.)
  DateTime newTime(theTimeInSeconds + secondsToAdd);

  // send the updated DateTime value back to the caller
  return newTime;
}

void setTheAlarm(DateTime alarmTime) {

  /*  Here is a copy of the declaration
   *  of the DS3231 library function
   *  that sets Alarm 1:
   *  
   *  void setA1Time(
   *    byte A1Day, 
   *    byte A1Hour, 
   *    byte A1Minute, 
   *    byte A1Second, 
   *    byte AlarmBits, 
   *    bool A1Dy, 
   *    bool A1h12, 
   *    bool A1PM
   *  ); 
  */

    // set the alarm to the new time
    clock.setA1Time(
      alarmTime.day(),
      alarmTime.hour(),
      alarmTime.minute(),
      alarmTime.second(),
      0x00001110, // this mask means alarm when seconds match
      false, false, false
    );

    // activate the alarm

    /* Tricks I learned about how the alarm on the DS3231 
     * signals an interrupt on the Arduino.
     * 
     * The alarm pin on the DS3231 is labeled, "SQW".
     * The DS3231 sends signals by changing the voltage
     * on its SQW pin. The voltaqge can be HIGH or LOW.
     * 
     * We want the voltage on SQW start as HIGH.
     * DS3231 signals an alarm by changing the signal to LOW.
     * 
     * Arduino's hardware can detect the change in the voltage
     * through digital pins having this special ability.
     * The hardware then literally interrupts the CPU,
     * causing it to switch over and run our special code.
     * 
     * A change from HIGH to LOW is called "FALLING".
     * The "attachInterrupt()" code in the setup() block
     * of this sketch tells Arduino to interrupt the CPU
     * when it detects a FALLING signal from the DS3231.
     * 
     * After that happens, the SQW pin will remain LOW.
     * The DS3231 cannot send any more "FALLING" signals
     * as long as the SQW pin remains LOW.
     * 
     * What this means to us as code writers is that
     * it is our job to tell the DS3231
     * to restore the HIGH voltage on its SQW pin.
     * We do this by "clearing" certain bits
     * in two control registers on the DS3231.
     * 
     * The DS3231 library provides code statements 
     * for this purpose. The trick, which I struggled
     * for a while to understand, is that the 
     * statement named "checkIfAlarm()" 
     * has a non-obvious side effect.
     * It clears one of those critical bits.
     * It is the only statement in the DS3231 library
     * that accomplishes this important step.
     * Which means we have to use it even if we do not
     * really need to check the status of the alarm.
     */
    clock.turnOffAlarm(1); // register 0Eh
    clock.checkIfAlarm(1); // register 0Fh

    // Now, we can turn on the alarm!
    clock.turnOnAlarm(1);
  
}

// this function gets called
// only after an interrupt occurs

void runTheSpecialCode() {
  
    // get the current time from the DS3231
    dt = RTClib::now(); // RTC class defined in DS3231.h

    // print the current time
    Serial.print(dt.hour()); Serial.print(":");
    if (dt.minute() < 10) Serial.print("0");
    Serial.print(dt.minute()); Serial.print(":");
    if (dt.second() < 10) Serial.print("0");
    Serial.println(dt.second());  

/* Set a new alarm to signal
 * ten seconds in the future
 * from the present time
 */
    setTheAlarm(addSecondsToTime(10, dt));
    
} // end of the special code
