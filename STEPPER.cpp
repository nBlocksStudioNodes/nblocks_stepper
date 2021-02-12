#include "STEPPER.h"

nBlock_STEPPER::nBlock_STEPPER(PinName MOSI, PinName MISO, PinName SCK, PinName pinSS, 
                               PinName pinSTEP, PinName pinDIR, PinName pinEN, PinName pinSTOP,
                               float speed, uint32_t accel, uint8_t axis, bool TMC2130): 
                               _step(pinSTEP), _dir(pinDIR), _en(pinEN), _stop(pinSTOP),         // instantiation so can use _en =1;
                               _speed(speed), _accel(accel), _axis(axis), _tmc2130(TMC2130) {   
    
    if(_speed < 0.00002) _speed = 0.00002;    // max 50KHz for Step pin frequency, LESS IS BLOCKING THE SYSTEM
    if(_speed > 1.1) _speed = 1.0;            // min 1Hz
    (this->_motion_ticker).attach(callback(this, &nBlock_STEPPER::_motion_tmrISR), _speed); // Instantiate the _motion_timer

        _stopInt = new InterruptIn(pinSTOP);       // instantiation of the Interrupt with 'new operator' 
        _stopInt->rise(callback(this, &nBlock_STEPPER::stopISR)); // call rise function of the InterruptIn class to assign a callback to ISR

    
    if (TMC2130) {                      // Have to check TMC2130 first, so need to use the 'new operator'....
        _spi = new SPI(MOSI, MISO, SCK);   // ... since we can't instantiate _spi and _ss before the { , like the rest of the functions.
        _spi->format(8,3);              // 8-bit format, mode 0,0
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
			case 4:					// Allow Motion
				_motion = MOTIONHALT;
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

uint8_t nBlock_STEPPER::write_TMC2130(uint8_t reg, uint32_t data) {
    uint8_t status;
    _ss ->write(0);                         // Set SPI Select Low
    status = _spi->write(reg);              // 40 bit Datagram, first     8 bits register-address, also returns status byte
    _spi->write((data >> 24UL) & 0xFF);     // 4 transactions x 8 bits = 32 bits data
    _spi->write((data >> 16UL) & 0xFF);
    _spi->write((data >>  8UL) & 0xFF);
    _spi->write((data >>  0UL) & 0xFF);
    _ss->write(1); 
    return status;                         // Set SPI Select High
}

uint8_t nBlock_STEPPER::read_TMC2130(uint8_t reg, uint32_t *data) {  
    uint8_t status;
    //write_TMC2130(reg, 0UL);          //set read address ?? does not comply with datasheet
    _ss->write(0);                      // Set CS Low   
    status = _spi->write(reg);          // send the register-address and get 8 bit SPI status
    *data = _spi->write(0x00)&0xFF;     // get 32 more bits to complete the 40 bit transaction
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;
    *data = _spi->write(0x00)&0xFF;
    *data <<=8;            
    _ss->write(1);                         // Set CS High
    return status;
}

void nBlock_STEPPER::init_TMC2130() {
    _ss->write(1);
    //write_TMC2130(WRITE_FLAG|REG_THIGH,      0x0000FFFFUL); // UPPER THRESHOLD FOR OPERATION WITH COOLSTEP AND STALLGUARD, TO BE REMOVED???
    write_TMC2130(WRITE_FLAG|REG_TCOOLTHRS,  0x0000007EUL);   // 04E 7E this was tested before SGT was 'discovered' and has low impact on STALL detection, TO BE REMOVED???
    write_TMC2130(WRITE_FLAG|REG_GCONF,      0x00002101UL);   // AIN curr ref, diag0_error,diag1_stall,
    write_TMC2130(WRITE_FLAG|REG_IHOLD_IRUN, 0x00001400UL);   // IRUN 0x0000 TO 0x1F00,  IHOLD=0x00 TO 0x1F, 
    write_TMC2130(WRITE_FLAG|REG_CHOPCONF,   0x02008008UL);   // MRES=[0=256ustep, 1=128, 2=64, 3=32,4=16,5=8,6=6,7=2,8=FULLSTEP], TBL1=24, TOFF=8
    write_TMC2130(WRITE_FLAG|REG_COOLCONF,   0x00049000UL);   // SGT = 0x0004E000UL : 7bit, signed value controls STALLGUARD2 LEVEL
    
}

void nBlock_STEPPER::stop(void) {
	_en = 0;
    _motion_tmr.stop();
     _motion = MOTIONHALT;    
}
  
void nBlock_STEPPER::turnLeft(void) {
    _en  = 0;
    _dir = 0;
    SteppingCounter = 0;  // 1000000/50KHz = 20sec of movement if the fastest speed is used
    if(_motion == MOTIONSTOP) _motion = MOTIONACTIVE;     
}
 
void nBlock_STEPPER::turnRight(void) {
    _en  = 0;
    _dir = 1;
    SteppingCounter = 0; // 1000000/50KHz = 20sec of movement if the fastest speed is used
    if(_motion != MOTIONSTOP) _motion = MOTIONACTIVE;

}

void nBlock_STEPPER::brake(void) {
    _motion_tmr.stop();
    if(_motion != MOTIONSTOP) _motion = MOTIONBRAKE;    
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
        //wait_us(10);   // 7.5us pulse with wait_us(1)
        _step = 0;
        SteppingCounter++;
        if(SteppingCounter == distance) {
            _motion_tmr.stop();
            _motion = MOTIONCOMPLETE;
        }
	}
}
