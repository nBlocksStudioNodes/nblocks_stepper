#include "TMC2130.h"

nBlock_TMC2130::nBlock_TMC2130(PinName MOSI, PinName MISO, PinName SCK, PinName pinCS, uint16_t Brightness, uint16_t ScanLimit):
    _spi(MOSI, MISO, SCK), _cs(pinCS)  {
    _spi.format(8,0);                // 8-bit format, mode 0,0
    _spi.frequency(1000000);         // SCLK = 1 MHz
    _cs = 1;

	// Reset flag
	must_update = 0;
    init_TMC2130(Brightness, ScanLimit);		
}

void nBlock_TMC2130::triggerInput(nBlocks_Message message){
	if (message.inputNumber == 0) {
        Value1 = message.intValue;       // Get Value from the 1st Input		
		Position1 = 1;
	}
    if (message.inputNumber == 1) {
        Value2 = message.intValue;       // Get Value from the 2nd Input		
		Position2 = 1;
    }
}

void nBlock_TMC2130::endFrame(void){
	if (Position1) {
		Position1 = 0;
		TMC2130_write1(Value1);
    }
	if (Position2) {
		Position2 = 0;
		TMC2130_write2(Value2);
	}	
}

void nBlock_TMC2130::write_TMC2130(uint8_t cmd, uint32_t data) {
    _cs = 0;                         // Set CS Low
    _spi.write(cmd);                 // Send address
    _spi.write((data>>24UL)&0xFF)&0xFF;
    _spi.write((data>>16UL)&0xFF)&0xFF;
    _spi.write((data>> 8UL)&0xFF)&0xFF;
    _spi.write((data>> 0UL)&0xFF)&0xFF;
    _cs = 1;                         // Set CS High
}

uint8_t nBlock_TMC2130::read_TMC2130(uint8_t cmd, uint32_t *data) {
    
    uint8_t s;
    write_TMC2130(cmd, 0UL);         //set read address
    _cs = 0;                         // Set CS Low   
    s     = _spi.write(cmd);
    *data = _spi.write(0x00)&0xFF;
    *data <<=8;
    *data = _spi.write(0x00)&0xFF;
    *data <<=8;
    *data = _spi.write(0x00)&0xFF;
    *data <<=8;
    *data = _spi.write(0x00)&0xFF;
    *data <<=8;            
    _cs = 1;                         // Set CS High
    return s;
}

void nBlock_TMC2130::init_TMC2130(uint8_t cmd, uint32_t data) {
    _cs = 1;                               // CS initially High 
//  write_TMC2130(WRITE_FLAG|REG_GCONF,      0x00000001UL); //voltage on AIN is current reference 
    write_TMC2130(WRITE_FLAG|REG_GCONF,      0x00000181UL); //AIN curr ref, diag0_error,diag1_stall,
    write_TMC2130(WRITE_FLAG|REG_IHOLD_IRUN, 0x00001010UL); //IHOLD=0x10, IRUN=0x10
    write_TMC2130(WRITE_FLAG|REG_CHOPCONF,   0x00008008UL); //native 256 microsteps, MRES=0, TBL1=24, TOFF=8

}