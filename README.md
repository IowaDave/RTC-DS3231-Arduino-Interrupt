# RTC-DS3231-Arduino-Interrupt
Tricks I learned about using a DS3231 Real Time Clock to interrupt an Arduino
## Problem
You have some skill and experience writing code with the Arduino IDE. You want to determine precise times when the Arduino will run special code segments in a sketch, without using the timers and counters in the Arduino hardware. In other words, would would like to avoid using code statements such as ```delay()```. 

Instead, you would like to use a very accurate DS3231 Real Time Clock module as a source of external interrupts. It may be important to you that the DS3231 can use a battery to maintain accurate time even if the Arduino temporarily loses power. 

Finally, you want to learn how to use the DS3231.h library by Andrew Wickert that is referred to in the Arduino online reference: [https://www.arduino.cc/reference/en/libraries/ds3231/](https://www.arduino.cc/reference/en/libraries/ds3231/). It contains a few tricks that can prove well worth the effort to master. You can import this library into your Arduino IDE by using the Library Manager (Tools > Manage Libraries...).

![The Library Manager](https://github.com/IowaDave/RTC-DS3231-Arduino-Interrupt/blob/main/images/Library.jpg)

## Solution
Go step by step. This tutorial demonstrates the following steps:
1. Put the special code segments into a block that runs only when it is allowed to, rather than in the main loop of your sketch. 
2. Connect the alarm pin (named SQW) of the DS3231 module to a pin on the Arduino that can be used for interrupts.
3. Set an alarm on the DS3231 clock.
4. The Arduino's main loop can ignore the clock while it performs other tasks.  
5. When the alarm happens, the DS3231 will pull the Arduino pin "low", that is, to near-zero voltage. 
6. The Arduino can detect this change and then "interrupt" the main loop to run the special code at that time.

If you want the special code to run more than once, at intervals you specify, your code can do Step 3 again and set a new alarm. The Solution in this example interrupts the Arduino repeatedly, at 10-second intervals.

## Resources
This tutorial draws from <span>&ldquo;official&rdquo;</span> references, including:
* the DS3231 datasheet:
[https://datasheets.maximintegrated.com/en/ds/DS3231.pdf](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf),
* the files named DS3231.h and DS3231.cpp in Andrew Wickert's public github repository:
[https://github.com/NorthernWidget/DS3231](https://github.com/NorthernWidget/DS3231),
* and the Arduino Reference for the attachInterrupt() function:
[https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/).

## Step 1: Special Code Segment
The special code in this example is trivial: it only prints out the current time from the DS3231. A real-life example might do something useful like water a plant or log a temperature measurement. Whatever the task is, the code should go into its own, special block to be run only when the DS3231 alarm interrupts the Arduino. 

I believe it is a good practice to break code into short functions, where each function handles only one task or one set of related tasks. A function's name can be anything; why not make it describe what the function does? Here's part of my function that runs the special code in this example.

```
void runTheSpecialCode() {
  // get the current time
  // using the DateTime and RTClib classes
  // defined in DS3231.h
  DateTime dt = RTClib::now();

  // print the current time
  Serial.print(dt.hour()); Serial.print(":");
  if (dt.minute() < 10) Serial.print("0");
  Serial.print(dt.minute()); Serial.print(":");
  if (dt.second() < 10) Serial.print("0");
  Serial.println(dt.second());  
    
  // There will be more to do here, as you will see.
  // This is enough, for now, to illustrate the idea: 
  // put special code in its own, special function
} 
```

## Step 2: Connections
You will run wires between five pairs of pins. Each pair performs one electrical purpose and matches a pin on the Arduino with a corresponding pin on the DS3231. Take it slowly, connect each pair, then check both ends to make sure each wire goes where it should.  The table lists the pairs in order as they attach, going left to right, onto the DS3231 from an Arduino Uno.

|Purpose|DS3231 Pin|Arduino Pin|
|--------|---------|----------|
|Alarm|SQW|3\*|
|SCL|SCL|SCL\*\*|
|SDA|SDA|SDA\*\*|
|5 volt power|VCC|5V|
|Ground|GND|GND|

* \*Pins 2 and 3 are the interrupt pins on the Uno. 
    * Use pins 0, 1, or 7 for a Leonardo; 7 is probably best.
    * Technically, pins 2 and 3 can also handle interrupts on a Leonardo. However, the DS3231 needs to use them for its communications.
* \*\*SCL and SDA are labeled as such on newer Arduinos. 
    * On older Unos lacking these markings, SCL is analog pin A5 and SDA is analog pin A4.
    * On Leonardos, SCL is digital pin 3 and SDA is digital pin 2.

Like I said, take your time making these connections. Slow and sure is often the fastest way to complete anything correctly.

## Step 3: Set an alarm on the DS3231
This step assumes that you have previously set the actual time on the DS3231. The example sketch contains code you can use to set the time on clock, in case you need it. Simply remove the comment delimiters, /\* and \*/, that surround it.

A DS3231 makes two, different alarms available: Alarm #1 (A1) and Alarm #2 (A2). Both alarms can be specified to a day and time, down to a minute. The difference is that A1 can be set down to a second. Each alarm has its own pair of functions in the DS3231 library for setting the time of the alarm and for reading that time. They are:

> setA1Time(),
> getA1Time(),
> setA2Time(), and
> getA2Time()

The setA1Time() function takes eight parameters:

```void setA1Time(byte A1Day, byte A1Hour, byte A1Minute, byte A1Second, byte AlarmBits, bool A1Dy, bool A1h12, bool A1PM); ```

The first five parameters are of type "byte". The C++ Standard defines the byte type this way [https://en.cppreference.com/w/cpp/types/byte](https://en.cppreference.com/w/cpp/types/byte):

> std::byte is a distinct type that implements the concept of byte as specified in the C++ language definition.

> Like char and unsigned char, it can be used to access raw memory occupied by other objects (object representation), but unlike those types, it is not a character type and is not an arithmetic type. A byte is only a collection of bits, and the only operators defined for it are the bitwise ones.

We can think of byte-type variables as if they were unsigned integers in this particular situation. They can hold an integer value between 0 and 255. *CAUTION: the code writer has to avoid nonsensical values. For example, a value of 102 makes no sense for any of these parameters. It is your job as the code writer to supply sensible values.*

Suppose you want to set an alarm for the 27th day of the month, at 17 seconds past 10:42 in the morning? The listing below shows how you might supply those values into the function. Each parameter is listed on its own line, to make them more readable by humans and to allow space for comments. The example here is incomplete; it demonstrates only the byte-type values for date and time. The function requires more parameters, as described below, and will not run in the form shown here.

``` 
    setA1Time(
      27, // the 27th day
      10, // the hour of 10
      42, // the 42nd minute of the hour
      17, // the 17th second of the minute
      // ... the remaining parameters are explained below
    );
```

The next parameter is a byte that truly is only a collection of bits. The bits have names as defined in the DS3231 datasheet (on page 11). 

|Bit 7|Bit 6|Bit 5|Bit 4|Bit 3|Bit 2|Bit 1|Bit0|
|--|--|--|DyDt|A1M4|A1M3|A1M2|A1M1|

Together, the bits form a "mask", or pattern, which tells the DS3231 when and how often to signal an alarm. A table on page 12 of the datasheet gives the meaning for different collections of the bits. Based on that table, the example sketch in this tutorial uses the following collection of bits:


|--|--|--|DyDt|A1M4|A1M3|A1M2|A1M1|
|0|0|0|0|1|1|1|0|

This arrangement of bits can be expressed explicitly in the code: ```0x00001110```. It tells the DS3231 to signal the alarm whenever "the seconds match", that is, when the "seconds" value of the alarm setting matches the "seconds" value of the current time. 

The final three parameters of the ```setA1Time()``` function are boolean, or true/false values. They tell the DS3231 more information about how to evaluate the alarm setting. The following code segment shows the completed call to setA1Time(), continuing the example shown above:

``` 
    setA1Time(
      27, // A1Day = the 27th day
      10, // A1Hour = the hour of 10 in the morning
      42, // A1Minute = the 42nd minute of the hour
      17, // A1Second = the 17th second of the minute
      0x00001110, // AlarmBits = signal when the seconds match
      false, // A1Dy false = A1Day means the date in the month; true = A1Day means the day of the week
      false, // A1h12 false = A1Hour of 0..23; true = A1Hour of 1..12 AM or PM
      false  // A1PM false = match A1Hour a.m.; true = match A1Hour p.m.
    );
```

In the example sketch, where we set the alarm to interrupt every 10 seconds, only the A1Second and the AlarmBits parameters matter. However, we need to supply them all when we call the function to setA1Time(). Correct values are no more difficult to provide than junk values are; we might as well exercise care with them.

The setA2Time() function works similarly, but without a parameter for seconds. Take some time to review lines 119 through 145 of the DS3231.h file in the library and pages 11-12 in the datasheet. Sit patiently with these references until you have found in them the information you need to understand and to use the alarm-setting functions.


