# SilentSTEPPER

Control Node for Stepping Motor driver IC TMC2130 supporting Stall Detection. 

<p align="center">
<img
src="img/01.PNG"
width = 300
/>
</p>

----

 *  Category:Motor
 *  HAL: mbed
 *  Tested: with LPC1768 and TMC2130 Silentstepstick
 *  Author: N. Chalikias

## Implementation Details

### L298 compatibility
The L298 commands are implemented as in the Input section below for input2 (Schematic pin 1).

### Gcode commands
Gcodes G0 and G1 are implemted for input 2 (Schematic pin 2).

### Stall Detection
By configuring registers TCOOLTHRS and GCONF, the TMC2130 DIAG1 pin is set to signal the Stall condition. The microprocessor pin connected to TMC2130 STALL pin is configured to create an interrupt.  
A Timer is set to the desired stepping frequency. A Timer ISR is attached to the Timer. The Timer_ISR pulses the STEP-Pin and increments a SteppingCounter. The DIAG1 Interrupt ISR Stops the Timer (this stops the movement) and Captures the SteppingCounter value to a StallPosition parameter.  
The next endFrame, outputs the StallPosition to the Node output creating a stall event.

### Accessing TNC2130 Registers
Registers are accesed with 40bit SPI transactions, sending a 40 bit command and getting back 40 bit status. 


## Input 

 * (Schematic pin 1) integer: Value
    * 0 or 0x30 STOP  IN1=0, IN2=0
    * 1 or 0x31 RIGHT IN1=1, IN2=0  STATE MACHINE: ACTIVATED ONLY IF IN STOP
    * 2 or 0x32 LEFT  IN1=0, IN2=1  STATE MACHINE: ACTIVATED ONLY IF MOVING RIGHT
    * 3 or 0x33 BRAKE IN1=1, IN2=1

## Output
 *  (Schematic pin 2) 
    * int: StallPossition

## Parameters:

 *  PinName: pinMOSI 
 *  PinName: pinMISO 
 *  PinName: pinSCK
 *  PinName: pinSS
 *  PinName: pinSTEP
 *  PinName: pinDIR
 *  PinName: pinENABLE
 *  PinName: pinDIAG1
 *  uint32_t: speedDefault
 *  char8_t: Axis the Node executes Gcode for (X,Y,Z,E,A,B,C,D) 

## Example:

[Ticker]-->[Counter]-->[SilentSTEPPER]





