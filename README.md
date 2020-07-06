# stopwatch_7Segment

##A project using the ARM microcontroller NXP LPC 802.

It is done by implementing a timer interrupt and a GPIO interrupt. The stopwatch should have 3 states, namely standby mode, run mode and a stop mode. These are to be toggled by pressing a button which is pre-programmed as an interrupt input pin. In the standby mode, the stopwatch should display only 00.00 on the 7-segment display. In the run mode, the microcontroller will increase the counter every 100ms and display the value of the counter. On the display, the value of the tenth, ones and hundreds of the counted time are to be seen. The counting is done by a 10Hz timer which was programmed to trigger an interrupt and enter the interrupt service routine every 100ms. The stopwatch will continue to count for 99.9 seconds before starting over from 00.0s. When the button is pressed, the stopwatch is stopped and display only the stopped time until the next button press, which will then reset the stopwatch and go into standby mode.


##Hardware:
4 digits 7-segment display SH5461AS is used for this project. The display has 12 pins: 8 anodes of the LEDs for segments a-g plus a decimal point, 4 pins for the cathode of each digit. 
![Schematics of the 7-segment]()     ![Segments A-G with a decimal point]()
In order for a specific segment to light up, the anode pin needs to be supplied with a positive voltage and the cathode with ground. Each pin was connected to a GPIO pin on the LPC 802 board so that the microcontroller could control its output by giving out either a binary 1 (3.3V) or a binary 0 (0V) at a specific time.
![Pin connections to the board]()

##Software
###Definition:
The code is written in C using the MCUXpresso IDE. The values to be written in the byte pin registers are defined as below:
![yte pin registers table]()

###Initialization/Configuration:
All interrupts were disabled in the beginning of the init() function to allow for the setup of GPIO pins and the SysTick timer. GPIO interrupt channel 0 was set to GPIO8. Finally, the GPIO and timer interrupts were enabled before the global interrupt was enabled.

###display function:
To display the values on the 7-segment was relatively easy and direct, whereas multiplexing the 4 digits was the first problem I faced. Since there were no latches as hardware available to control each digit separately, a software solution must be implemented. The solution to that is by switching on one digit at a time, allowing the value to be shown for a small amount of time (with a small delay function) before switching it off again and switching on the next digit. The process repeats in a continuous while loop. The microprocessor is able to execute all these commands fast enough that it is perceived by the human eye as though all digits are lighted up simultaneously.

###main function:
Two global variables were declared for the interrupt service routine to interact with the main function. The SysTick ISR is called every 100ms to increase the ‘counter’, which will then be used in the main function to display the counter value on the 7-segment. Whereas the ISR for the user button on the board changes the value of the variable ‘state’ when called. By using a switch-case statement in the main, different set of commands will be executed depending on the value of ‘state’. When ‘state’ = 0, the program keeps the counter at 0 and displays only 0 on the 7-segment. In the instance that the button is pressed, the ISR is called and now the value of ‘state’ would be incremented to 1. The program now allows the counter to be increased by the timer ISR every 100ms and updates the display constantly. When ‘state’ = 2, the value of counter is captured, and this value will be permanently displayed until ‘state’ goes back to 0.
