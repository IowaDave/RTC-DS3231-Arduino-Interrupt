# RTC-DS3231-Arduino-Interrupt
Tricks I learned about using a DS3231 Real Time Clock to interrupt an Arduino
## Problem
You want to determine precise times when the Arduino will run special code segments in your sketch, without using the timers and counters in the Arduino hardware. In other words, to avoid using code statements such as ```delay()```. Instead, you would like to use a very accurate DS3231 Real Time Clock module as a source of external interrupts. And you want to learn how to use the DS3231.h library by Andrew Wickert that is referred to in the Arduino online reference: [https://www.arduino.cc/reference/en/libraries/ds3231/](https://www.arduino.cc/reference/en/libraries/ds3231/).

## Solution
Go step by step:
1. Put the special code segments into a block that runs only when it is allowed to, rather than in the main loop of your sketch. 
2. Set an alarm on the DS3231 clock, then connect the clock's alarm pin to a pin on the Arduino that can be used for interrupts.
3. The Arduino's main loop can ignore the clock while it performs other tasks.  
4. When the alarm happens, the DS3231 will pull the Arduino pin "low", that is, to near-zero voltage. 
5. The Arduino can detect this change and then "interrupt" the main loop to run the special code at that time.

If you want the special code to run more than once, at intervals you specify, your code can do Step 2 again and set a new alarm. The Solution in this example interrupts the Arduino repeatedly, at 10-second intervals.

## Resources
This tutorial draws from &ldquo;official&rdquo; references, including:
* the DS3231 datasheet:
[https://datasheets.maximintegrated.com/en/ds/DS3231.pdf](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf),
* the files named DS3231.h and DS3231.cpp in Andrew Wickert's github repository:
[https://github.com/NorthernWidget/DS3231](https://github.com/NorthernWidget/DS3231),
* and the Arduino Reference for the attachInterrupt() function:
[https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/).




The special code in this example is trivial: it only prints out the current time from the DS3231. A real-life example might do something useful like water a plant or log a temperature measurement. Whatever the task is, the code should go into its own, special block to be run only when the DS3231 alarm interrupts the Arduino. 

```
