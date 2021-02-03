#include "STEPPER.h"

nBlock_STEPPER::nBlock_STEPPER(PinName MOSI, PinName MISO, PinName SCK, PinName pinSS, 
                               PinName pinSTEP, PinName pinDIR, PinName pinEN, PinName pinSTOP,
                               uint32_t speed, uint32_t accel, char8_t axis, bool TMC2130): 
                               _step(pinSTEP), _dir(pinDIR), _en(pinEN), _stop(pinSTOP),         // instantiation so can use _en =1;
                               _speed(speed), _accel(accel), _axis(axis), _tmc2130 (TMC2130) {   
    
    if (TMC2130) {                      // Have to check TMC2130 first, so need to use the 'new' operator ....
        _spi = new (MOSI, MISO, SCK);   // ... since we can't instantiate _spi before the {
        _spi->format(8,0);              // 8-bit format, mode 0,0
        _spi->frequency(1000000);       // SCLK = 1 MHz
        _ss =  new (pinCS);             // instantiation with 'new' operator 
        _ss -> write(1);                // use with pointer
        init_TMC2130();
    }
}

void nBlock_STEPPER::triggerInput(nBlocks_Message message){
	if (message.inputNumber == 0) {
        Value1 = message.intValue;       // Get Value from the 1st Input		
		Position1 = 1;
	}
    if (message.inputNumber == 1) {
        Value2 = message.intValue;       // Get Value from the 2nd Input		
		Position2 = 1;
    }
}

void nBlock_STEPPER::endFrame(void){
	if (Position1) {
		Position1 = 0;

		char buf[1];
		buf[0] = received_value;
		switch (buf[0]) {
			case 0:					// stop uncoditionally
				state = 0;
				stop();
				break;
			case 0x30:				// stop uncoditionally
				state = 0;
				stop();
				break;				
			case 1:
				if (state == 0){	//move right only if is in stop
					state = 1;
					turnRight();
					}
				break;
			case 0x31:
				if (state == 0){	//move right only if is in stop
					state = 1;
					turnRight();
					}
				break;				
			case 2:
				if (state == 1){	//turn left only if moving right
					state = 2;
					turnLeft();
					}
				break;
			case 0x32:
				if (state == 1){	//turn left only if moving right
					state = 2;
					turnLeft();
					}
				break;				
			case 3:					// brake unconditionally
				brake();
				break;
			case 0x33:				// brake unconditionally
				brake();
				break;				
			default:				// any other input ignore
				break;
		}//switch

    }
	if (Position2) {
		Position2 = 0;
	}	
}

void nBlock_STEPPER::write_TMC2130(uint8_t cmd, uint32_t data) {
    _ss ->write(0);                   // Set CS Low
    _spi->write(cmd);                 // Send address
    _spi->write((data>>24UL)&0xFF)&0xFF;
    _spi->write((data>>16UL)&0xFF)&0xFF;
    _spi->write((data>> 8UL)&0xFF)&0xFF;
    _spi->write((data>> 0UL)&0xFF)&0xFF;
    _ss = 1;                         // Set CS High
}

uint8_t nBlock_STEPPER::read_TMC2130(uint8_t cmd, uint32_t *data) {
    
    uint8_t s;
    write_STEPPER(cmd, 0UL);         //set read address
    _ss = 0;                         // Set CS Low   
    s     = _spi->write(cmd);
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;            
    _ss = 1;                         // Set CS High
    return s;
}

void nBlock_STEPPER::init_TMC2130() {
    _ss = 1;                               // CS initially High 
//  write_STEPPER(WRITE_FLAG|REG_GCONF,      0x00000001UL); //voltage on AIN is current reference 
    write_STEPPER(WRITE_FLAG|REG_GCONF,      0x00000181UL); //AIN curr ref, diag0_error,diag1_stall,
    write_STEPPER(WRITE_FLAG|REG_IHOLD_IRUN, 0x00001010UL); //IHOLD=0x10, IRUN=0x10
    write_STEPPER(WRITE_FLAG|REG_CHOPCONF,   0x00008008UL); //native 256 microsteps, MRES=0, TBL1=24, TOFF=8
}

void nBlock_STEPPER::stop(void) {
  	_in1 = OFF; 
  	_in2 = OFF;
}
  
void nBlock_STEPPER::turnLeft(void) {
    _enable.write(0.95f);
	_in1 = OFF;
	_in2 = ON;      
}
 
void nBlock_STEPPER::turnRight(void) {
    _enable.write(0.95f);
    _in1 = ON;
    _in2 = OFF;   
}

void nBlock_STEPPER::brake(void) {
    _enable.write(0.95f);
    _in1 = ON;
    _in2 = ON;   
}