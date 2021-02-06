#include "STEPPER.h"

nBlock_STEPPER::nBlock_STEPPER(PinName MOSI, PinName MISO, PinName SCK, PinName pinSS, 
                               PinName pinSTEP, PinName pinDIR, PinName pinEN, PinName pinSTOP,
                               float speed, uint32_t accel, uint8_t axis, bool TMC2130): 
                               _step(pinSTEP), _dir(pinDIR), _en(pinEN), _stop(pinSTOP),         // instantiation so can use _en =1;
                               _speed(speed), _accel(accel), _axis(axis), _tmc2130(TMC2130) {   
    
    if(_speed < 0.00002) _speed = 0.00002;    // max 50KHz for Step pin frequency
    if(_speed > 1.1) _speed = 1.0;            // min 1Hz
    (this->_motion_ticker).attach(callback(this, &nBlock_STEPPER::_motion_tmrISR), _speed); // Instantiate the _motion_timer

        _stopInt = new InterruptIn(pinSTOP);       // instantiation of the Interrupt with 'new operator' 
        _stopInt->rise(callback(this, &nBlock_STEPPER::stopISR)); // call rise function of the InterruptIn class to assign a callback to ISR

    
    if (TMC2130) {                      // Have to check TMC2130 first, so need to use the 'new operator'....
        _spi = new SPI(MOSI, MISO, SCK);   // ... since we can't instantiate _spi and _ss before the { , like the rest of the functions.
        _spi->format(8,0);              // 8-bit format, mode 0,0
        _spi->frequency(1000000);       // SCLK = 1 MHz
        _ss =  new DigitalOut(pinSS);   // instantiation with 'new operator' 
        _ss->write(1);                  // use with pointer

        init_TMC2130();                 // Configure TMC2130 DIAG1 pin to be Stall detection output
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
		buf[0] = Value1;
		switch (buf[0]) {
			case 0:					// stop uncoditionally
				_state = 0;
				stop();
				break;
			case 0x30:				// stop uncoditionally
				_state = 0;
				stop();
				break;				
			case 1:
				if ((_state == 0) & (_motion != MOTIONSTOP)){	//move right only if is in stop
					_state = 1;
					turnRight();
					}
				break;
			case 0x31:
				if (_state == 0){	//move right only if is in stop
					_state = 1;
					turnRight();
					}
				break;				
			case 2:
				if ((_state == 1) & (_motion != MOTIONSTOP)) {	//turn left only if moving right
					_state = 2;
					turnLeft();
					}
				break;
			case 0x32:
				if (_state == 1){	//turn left only if moving right
					_state = 2;
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
    if(_motion == MOTIONSTOP) {
        output[0] = stopPosition;
        available[0] = 1;
    }
}

void nBlock_STEPPER::write_TMC2130(uint8_t cmd, uint32_t data) {
    _ss ->write(0);                   // Set CS Low
    _spi->write(cmd);                 // Send address
    _spi->write((data>>24UL)&0xFF)&0xFF;
    _spi->write((data>>16UL)&0xFF)&0xFF;
    _spi->write((data>> 8UL)&0xFF)&0xFF;
    _spi->write((data>> 0UL)&0xFF)&0xFF;
    _ss->write(1);                         // Set CS High
}

uint8_t nBlock_STEPPER::read_TMC2130(uint8_t cmd, uint32_t *data) {
    
    uint8_t s;
    write_TMC2130(cmd, 0UL);         //set read address
    _ss->write(0);                         // Set CS Low   
    s     = _spi->write(cmd);
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;            
    _ss->write(1);                         // Set CS High
    return s;
}

void nBlock_STEPPER::init_TMC2130() {
    _ss->write(1);                               // CS initially High 
//  write_STEPPER(WRITE_FLAG|REG_GCONF,      0x00000001UL); //voltage on AIN is current reference 
    write_TMC2130(WRITE_FLAG|REG_GCONF,      0x00000181UL); //AIN curr ref, diag0_error,diag1_stall,
    write_TMC2130(WRITE_FLAG|REG_IHOLD_IRUN, 0x00001010UL); //IHOLD=0x10, IRUN=0x10
    write_TMC2130(WRITE_FLAG|REG_CHOPCONF,   0x00008008UL); //native 256 microsteps, MRES=0, TBL1=24, TOFF=8
}

void nBlock_STEPPER::stop(void) {
	_en = 0;
    _motion_tmr.stop();
    // _motion = MOTIONHALT;    
}
  
void nBlock_STEPPER::turnLeft(void) {
    _en  = 1;
    _dir = 0;
    SteppingCounter = 1000000;  // 1000000/50KHz = 20sec of movement if the fastest speed is used
    if(_motion != MOTIONSTOP) _motion = MOTIONACTIVE;     
}
 
void nBlock_STEPPER::turnRight(void) {
    _en  = 1;
    _dir = 1;
    SteppingCounter = 1000000; // 1000000/50KHz = 20sec of movement if the fastest speed is used
    if(_motion != MOTIONSTOP) _motion = MOTIONACTIVE;

}

void nBlock_STEPPER::brake(void) {
    _motion_tmr.stop();
     _motion = MOTIONBRAKE;    
}

void nBlock_STEPPER::stopISR() {
    if(_motion == MOTIONACTIVE){
		_motion_tmr.stop();
        stopPosition = SteppingCounter;
        _motion = MOTIONSTOP;
	}
}

void nBlock_STEPPER::_motion_tmrISR() {
    if(_motion == MOTIONACTIVE){
		_step = 1;      // 100ns pulse for a single command
        _step = 1;      // 
        _step = 1;      // 250ns pulse with 3 x commands
        _step = 1;      // 
        _step = 1;      // 
        _step = 1;      // 600ns pulse with 6x commands, but compiler optimization might affect this
        //wait_us(1);   // 7.5us pulse with wait_us(1)
        _step = 0;
        SteppingCounter--;
        if(SteppingCounter == 0) {
            _motion_tmr.stop();
            _motion = MOTIONCOMPLETE;
        }
	}
}
