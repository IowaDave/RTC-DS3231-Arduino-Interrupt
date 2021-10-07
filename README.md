# RTC-DS3231-Arduino-Interrupt
![DS3231 module with Arduino](https://github.com/IowaDave/RTC-DS3231-Arduino-Interrupt/blob/main/images/hardware.jpg)
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

<p>This tutorial draws from &ldquo;official&rdquo; references, including:</p>

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

We will talk to the DS3231 module by means of a DS3231 "object", a kind of software toolbox with a name on it. The DS3231 library defines a lot of functions -- think of them as tools -- inside the box. When we want to use a function, we write the name of the toolbox, followed by a dot, then the name of the tool. The example sketch creates a "clock" variable for this purpose. Then the sketch can access the tools in the "clock" box. All of the tools are declared in the DS3231.h file, mentioned above.

```
#include <DS3231.h>
DS3231 clock;
Serial.println(clock.getMinute()); // the current minute, 0..59
```

We will use tools in our "clock" object to set the alarm. But first we need to calculate the alarm time.

### Calculating the alarm time
This step assumes that you have previously set the actual time on the DS3231. The example sketch contains code you can use to set the time on clock, in case you need it. Simply remove the comment delimiters, /\* and \*/, that surround it.

The example sketch in this tutoral computes an alarm time in the future by adding an interval, as a number of seconds, to the current time. The example adds ten seconds. A minute would add 60 seconds.  An hour would add 3,600 seconds. A day, 86,400 seconds. And so forth. 

The DS3231 library has a hidden "trick" that makes it easy to add time as a number of seconds. You will not find this trick listed among the available functions on the DS3231 library's README page. It is not entirely obvious by looking at the DS3231.h file, either. Some of the details wait to be found in the DS3231.cpp code file. Here are the steps to perform the computation.

1. Declare a DateTime object, another kind of software toolbox, using the default method.
2. Set this variable equal to the ```now()``` function, which needs to be accesed in a special way.
3. Extract the value of the DateTime variable as an unsigned, long integer. The value represents a number of seconds.
4. Add the amount of seconds in the interval to this value. The sum is the new alarm time, in seconds.
5. Declare a new, different DateTime object, using an alternative method that takes that number of seconds as an initial value.

Steps 4 and 5 can be combined.

```
const Uint32_t interval = 10;  // number of seconds to add 
DateTime currentTime; // default declaration
currentTime = RTClib::now(); // RTClib is defined in DS3231.h
uint32_t currentSeconds = currentTime.unixtime(); // express the date in seconds
DateTime alarmTime(currentSeconds + interval); // add 10 seconds and create a new date
```

Even though the alarmTime object is created based on a number of seconds, it provides tools in its toolbox to express its year, month, day, hour, minute, and second. We set the alarm time on the DS3231, as described next, using the alarmTime as the source of the values we will need.

Suppose, for example, that the currentTime reported by the DS3231 module was 7 seconds past 10:42 in the morning on Wednesday, October 27, 2021. The alarmTime calculated above would be 10:42:17 that same day, ten seconds later.

### Set the alarm time

A DS3231 makes two, different alarms available: Alarm #1 (A1) and Alarm #2 (A2). Both alarms can be specified to a day and time, down to a minute. The difference is that A1 can be further specified down to a second. Each alarm has its own pair of functions in the DS3231 library for setting the time of the alarm and for reading that time. The functions are all accessed through a DS3231 object, for example, the one we named, "clock":

> clock.setA1Time(),
> clock.getA1Time(),
> clock.setA2Time(), and
> clock.getA2Time()

The setA1Time() function takes eight parameters:

```void setA1Time(byte A1Day, byte A1Hour, byte A1Minute, byte A1Second, byte AlarmBits, bool A1Dy, bool A1h12, bool A1PM); ```

The first five parameters are of type "byte". The C++ Standard defines the byte type this way [https://en.cppreference.com/w/cpp/types/byte](https://en.cppreference.com/w/cpp/types/byte):

> std::byte is a distinct type that implements the concept of byte as specified in the C++ language definition.

> Like char and unsigned char, it can be used to access raw memory occupied by other objects (object representation), but unlike those types, it is not a character type and is not an arithmetic type. A byte is only a collection of bits, and the only operators defined for it are the bitwise ones.

We can think of byte-type variables as if they were unsigned integers in this particular situation. They can hold an integer value between 0 and 255. *CAUTION: the code writer has to avoid nonsensical values. For example, a value of 102 makes no sense for any of these parameters. It is your job as the code writer to supply sensible values.*

Let's continue with the alarmTime created in the previous step: the 27th day of the month, at 17 seconds past 10:42 in the morning. The listing below shows how you might supply those values into the function. I list each parameter on its own line, to make them more readable by humans and to allow space for comments. The example here is incomplete; it demonstrates only the byte-type values for date and time. The function requires more parameters, as described below, and will not run in the form shown here.

By the way, notice that the "clock" and "alarmTime" variables are objects, that is, they are software toolboxes. As you see, we use tools from inside the respective toolboxes to access the information the objects contain.

``` 
    clock.setA1Time(
      alarmTime.day(), // the day of the month: 27
      alarmTime.hour(), // the hour of the day: 10
      alarmTime.minute(), // the minute of the hour: 42
      alarmTime.second(), // the second of the minute: 17
      // ... the remaining parameters are explained below
    );
```

The next parameter is a byte that truly is only a collection of bits. The bits have names as defined in the DS3231 datasheet (on page 11). 

|Bit 7|Bit 6|Bit 5|Bit 4|Bit 3|Bit 2|Bit 1|Bit 0|
|-----|-----|-----|-----|-----|-----|-----|-----|
|--|--|--|DyDt|A1M4|A1M3|A1M2|A1M1|

Together, the bits form a "mask", or pattern, which tells the DS3231 when and how often to signal an alarm. A table on page 12 of the datasheet gives the meaning for different collections of the bits. Based on that table, the example sketch in this tutorial uses the following collection of bits:


|--|--|--|DyDt|A1M4|A1M3|A1M2|A1M1|
|--|--|--|----|----|----|----|----|
|0|0|0|0|1|1|1|0|

This arrangement of bits can be expressed explicitly in the code: ```0x00001110```. It tells the DS3231 to signal the alarm whenever "the seconds match", that is, when the "seconds" value of the alarm setting matches the "seconds" value of the current time. 

The final three parameters of the ```setA1Time()``` function are boolean, or true/false values. They tell the DS3231 more information about how to evaluate the alarm setting. The following code segment shows the completed call to setA1Time(), continuing the example begun above:

``` 
    clock.setA1Time(
      alarmTime.day(), // the day of the month: 27
      alarmTime.hour(), // the hour of the day: 10
      alarmTime.minute(), // the minute of the hour: 42
      alarmTime.second(), // the second of the minute: 17
      0x00001110, // AlarmBits = signal when the seconds match
      false, // A1Dy false = A1Day means the date in the month; true = A1Day means the day of the week
      false, // A1h12 false = A1Hour of 0..23; true = A1Hour of 1..12 AM or PM
      false  // A1PM false = match A1Hour a.m.; true = match A1Hour p.m.
    );
```

In the example sketch, where we set the alarm to interrupt every 10 seconds, only the A1Second and the AlarmBits parameters matter. However, we need to supply them all when we call the function to setA1Time(). Correct values are no more difficult to provide than junk values are; we might as well exercise care with them.

The setA2Time() function works similarly, but without a parameter for seconds. Take some time to review lines 119 through 145 of the DS3231.h file in the library and pages 11-12 in the datasheet. Sit patiently with these references until you have found in them the information you need to set an alarm time.

### Enable the alarm

After you set the time, you still need to actually enable the alarm in the DS3231. I believe that it works best to follow a three-step process:

1. disable the alarm
2. clear the alarm status flag
3. enable the alarm

For the alarm A1, the instructions in the DS3231 library would be:

```
turnOffAlarm(1); // clear "enable" bit in register 0Eh
checkIfAlarm(1); // clear status bit in register 0Fh
turnOnAlarm(1); // set enable bit in register 0Eh
```

Why would code writers choose to "check" an alarm that they know is not presently sending a signal? The reason is that the ```checkIfAlarm()``` function has a non-obvious side effect. It clears the alarm status flag. This flag must be cleared in order for the DS3231 to place a HIGH voltage on its SQW alarm pin. When that bit is set (to 1), the voltage on SQW pin is LOW, and vice versa. Refer to the discussion of bits 1 and 0 in the "Status Register (0Fh)", on page 14 of the DS3231 datasheet.

## Step 4: Allow the Clock to Tell the Time
Your main loop has no need to measure time. It needs only to check a flag to see whether an alarm has happened. In the example sketch, this flag is a boolean variable named "alarmEventFlag":

```
if (alarmEventFlag == true) {
  // run the special code
}
```

Most of the time, the flag will be *false*, and the loop will skip over the special code.  How does the sketch set up the flag? Three steps:

1. Define the flag variable "globally" in the sketch, that is, up at the top in the main ".ino" file for the sketch, outside of any functions. Its initial value of *false* (no alarm yet) can be set at the same time, like this<br>```bool alarmEventFlag = false;```
2. Write a function that sets the flag to *true* when an interrupt happens. Such a function is called an Interrupt Service Routine, or ISR. The example sketch uses a descriptive name for an ISR designed to work with interrupts from the RTC:<br>```void rtcISR() {alarmEventFlag = true;}
3. Finally, the ```attachInterrupt()``` function provided by the Arduino IDE brings it all together. It tells the Arduino hardware to run the rtcISR() function immediately whenever it detects a "FALLING" signal on a designated digital pin.

What is a FALLING signal?  It means a change in voltage, to LOW from HIGH, as detected by the digital pin of the Arduino. Where does the voltage change come from? It originates on the alarm pin of the DS3231 module. That pin is labeled, SQW, and it emits a HIGH voltage, near the VCC supply level (i.e., 5 volts on the Uno,) most of the time. An alarm causes the DS3231 to change the voltage on the SQW pin to LOW. The Arduino senses the voltage coming from the SQW pin and notices the change. We tell the Arduino to notice FALLING because that event only happens once per alarm, whereas the LOW level can persist and confuse the Arduino into triggering many interrupts.

Which pin can sense a FALLING change in voltage? For Unos, you may choose either of pins 2 or 3. For Leonardo, it can be any one of pins 0, 1, or 7. (Yes, I know, Leonardo senses interrupts on pins 2 and 3 also. However, those are Leonardo's I2C pins, which means the DS3231 module would be using them. I start with pin 7 for interrupts on a Leonardo.) The example sketch defines a dataPin variable and initializes its value to 3 for running on an Uno this way:<br>```int dataPin = 3;```

It all comes together as follows. Notice that the ISR is referred to only by its name, without the parenthese or the curly braces:<br>```attachInterrupt(digitalPinToInterrupt(dataPin), rtcISR, FALLING);```

For deep and occult reasons, always use the special function, ```digitalPinToInterrupt()```, when specifing the pin number for an interrupt. I leave it as an exercise for the reader to discover why we need that function.

## Steps 5 and 6: What happens when the DS3231 signals an alarm
* The DS3231 will reduce the voltage on its SQW pin to LOW from HIGH. 
* The "dataPin" on the Arduino will detect a FALLING event and interrupt the Arduino. 
* This will cause the ISR to run immediately. 
* The ISR changes the alarmEventFlag to *true*. And that is all it needs to do.
* The next time the main loop tests the alarmEventFlag, it will find the flag to be true.
    * The special code will then run.
    * The code turns off the alarm and restores the value of *false* to the alarmEventFlag .

The special code can set a new alarm time also, if you want to repeat the cycle. Begin by calculating the new alarm time, as described in Step 3, and follow the sequence of steps from there.

I would expect the example sketch to produce output similar to the illustration below, when it is run on an Arduino Uno correctly connected to a DS3231 module, the way I describe it in this tutorial. Don't be surprised if the times displayed are different. You should work with your own times, anyway.

![Serial Monitor with sketch running](https://github.com/IowaDave/RTC-DS3231-Arduino-Interrupt/blob/main/images/SerialMonitor.jpg)


