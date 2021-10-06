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
2. Connect the alarm pin (named, SQW) of the DS3231 module to a pin on the Arduino that can be used for interrupts.
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
