#ifndef __NB_STEPPER
#define __NB_STEPPER


#define EN_PIN    7 //enable (CFG6)
#define DIR_PIN   8 //direction
#define STEP_PIN  9 //step

#define CS_PIN   10 //CS chip select
#define MOSI_PIN 11 //SDI/MOSI (ICSP: 4, Uno: 11, Mega: 51)
#define MISO_PIN 12 //SDO/MISO (ICSP: 1, Uno: 12, Mega: 50)
#define SCK_PIN  13 //CLK/SCK  (ICSP: 3, Uno: 13, Mega: 52)

// STEPPER registers
#define WRITE_FLAG     (1<<7) //write flag
#define READ_FLAG      (0<<7) //read flag
#define REG_GCONF      0x00
#define REG_GSTAT      0x01
#define REG_IHOLD_IRUN 0x10
#define REG_CHOPCONF   0x6C
#define REG_COOLCONF   0x6D
#define REG_DCCTRL     0x6E
#define REG_DRVSTATUS  0x6F



#include "nworkbench.h"

class nBlock_STEPPER: public nBlockSimpleNode<1> {
public:
    nBlock_STEPPER(PinName MOSI, PinName MISO, PinName SCK, PinName pinCS, uint16_t Brightness, uint16_t ScanLimit);
    void        triggerInput(nBlocks_Message message);
	void        endFrame();

	void        write_TMC2130(uint8_t cmd, uint32_t data);
    uint8_t     read_TMC2130 (uint8_t cmd, uint32_t *data);
    void        init_TMC2130 ();
    void        stop(void);
	void        turnRight(void);
	void        turnLeft(void);
	void        brake(void);

    uint32_t    Position1;
    uint32_t    Position2;
   
private:	
    SPI         * _spi;

    DigitalOut  * _cs;
    DigitalOut  _step;
    DigitalOut  _dir;
    DigitalOut  _en;
    DigitalOut  _stop;

	uint32_t    _speed;
    uint32_t    _accel;
    uint32_t    _axis;
    bool        _tmc2130;
};

#endif