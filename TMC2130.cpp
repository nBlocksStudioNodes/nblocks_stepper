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

void nBlock_TMC2130::TMC2130_write1(uint16_t Value1) {
    _cs = 1;                                // CS initially High 
    dig[3] = (Value1 / 1000);               // Encode Value and Position to digits
    dig[2] = (Value1 - dig[3] * 1000) / 100;
    dig[1] = (Value1 - dig[3] * 1000 - dig[2] * 100) / 10;
    dig[0] = (Value1 - dig[3] * 1000 - dig[2] * 100 - dig[1] * 10);
    // Send digit values to TMC2130
    spi_write_2bytes(0x01, dig[3]);         //digit 0 
    spi_write_2bytes(0x02, dig[2]);         //digit 1 
    spi_write_2bytes(0x03, dig[1]);         //digit 2 has active the decimal point
    spi_write_2bytes(0x04, dig[0]);         //digit 3 
}

void nBlock_TMC2130::TMC2130_write2(uint16_t Value2) {
    _cs = 1;                                // CS initially High 
    dig[7] = (Value2 / 1000);               // Encode Value and Position to digits
    dig[6] = (Value2 - dig[7] * 1000) / 100;
    dig[5] = (Value2 - dig[7] * 1000 - dig[6] * 100) / 10;
    dig[4] = (Value2 - dig[7] * 1000 - dig[6] * 100 - dig[5] * 10);
    // Send digit values to TMC2130
    spi_write_2bytes(0x05, dig[7]);         //digit 0 
    spi_write_2bytes(0x06, dig[6]);         //digit 1 
    spi_write_2bytes(0x07, dig[5]);         //digit 2 has active the decimal point
    spi_write_2bytes(0x08, dig[4]);         //digit 3 
}


void nBlock_TMC2130::spi_write_2bytes(unsigned char MSB, unsigned char LSB) {
    _cs = 0;                         // Set CS Low
    _spi.write(MSB);                 // Send two bytes
    _spi.write(LSB);
    _cs = 1;                         // Set CS High
}

void nBlock_TMC2130::init_TMC2130(uint16_t Brightness, uint16_t ScanLimit) {
    _cs = 1;                               // CS initially High 
    spi_write_2bytes(0x09, 0xFF);         // Decoding off //nikos changed to full decode
    spi_write_2bytes(0x0A, Brightness);         // Brightness to intermediate
    spi_write_2bytes(0x0B, ScanLimit);         // Scan limit 7th digit
    spi_write_2bytes(0x0C, 0x01);         // Normal operation mode, this is the shutdown register
    spi_write_2bytes(0x0F, 0x0F);         // Enable display test

    //wait_us(500000);                   // 500 ms delay

    spi_write_2bytes(0x01, 0x0F);         // Clear row 0.
    spi_write_2bytes(0x02, 0x0F);         // Clear row 1.
    spi_write_2bytes(0x03, 0x0F);         // Clear row 2.
    spi_write_2bytes(0x04, 0x0F);         // Clear row 3.
    spi_write_2bytes(0x05, 0x0F);         // Clear row 4.
    spi_write_2bytes(0x06, 0x0F);         // Clear row 5.
    spi_write_2bytes(0x07, 0x0F);         // Clear row 6.
    spi_write_2bytes(0x08, 0x0F);         // Clear row 7.
    spi_write_2bytes(0x0F, 0x08);         // Disable display test

}