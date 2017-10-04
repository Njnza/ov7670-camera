#include "IO_config.h"
#include "sensor.h"
#include <Wire.h>
#include "delay.h"

// serial stuff  
static const byte LF = 10; // line feed character

static const unsigned long _BAUDRATE = 115200;
HardwareSerial *serialPtr = &Serial;

static const uint8_t fW = 160;//80;
static const uint8_t fH = 120;//60;

static const uint8_t YUYV_BPP = 2; // bytes per pixel
static const unsigned int MAX_FRAME_LEN = fW * YUYV_BPP;
byte rowBuf[MAX_FRAME_LEN];
//boolean volatile bRequestPending = false;
char volatile bNewFrame = 0;

// *****************************************************
//                          SETUP
// *****************************************************
void setup()
{
  setup_IO_ports();
  
  serialPtr->begin(_BAUDRATE);

  //serialPtr->println("Initializing sensor...");
  for (int i = 0; i < 10; i ++) {
       unsigned int result = sensor_init();
      if (result != 0) {
        //serialPtr->print("inited OK, sensor PID: ");
        //serialPtr->println(result, HEX);
        break;
      }
      else if (i == 5) {
          serialPtr->println("PANIC! sensor init keeps failing!");
          while (1);
      } else {
          serialPtr->println("retrying...");
          delay(300);
      }
  }
  DISABLE_WREN; // disable writing to fifo
  attachInterrupt(VSYNC_INT, &vsyncIntFunc, FALLING);
  delay(100);
}
// *****************************************************
//                          LOOP
// *****************************************************
void loop()
{  
 
 
}

// *****************************************************
//               VSYNC INTERRUPT HANDLER
// *****************************************************
void __inline__ vsyncIntFunc() {
      
      if (bNewFrame==2) {
        DISABLE_WREN; // disable writing to fif
        detachInterrupt(VSYNC_INT);
        processRequest();
        bNewFrame = 0;
        //attachInterrupt(VSYNC_INT, &vsyncIntFunc, FALLING);
      }
      else if (bNewFrame == 0){
          
          ENABLE_WRST;
          //_delay_cycles(500);
          SET_RCLK_H;
          //_delay_cycles(100);
          SET_RCLK_L;
          DISABLE_WRST;
          //_delay_cycles(10);
          ENABLE_WREN; // enable writing to fifo
          bNewFrame = 1;
      }
      else {
          bNewFrame = 2;
        }
}

// **************************************************************
//                      PROCESS SERIAL REQUEST
// **************************************************************
void processRequest() {
	
	//fifo rrst
    ENABLE_RRST;
    //_delayNanoseconds(5);
    SET_RCLK_H;
    //_delayNanoseconds(5);
    SET_RCLK_L;
    DISABLE_RRST;
	//
  //DISABLE_WREN; // disable writing to fifo
	for (int i =0; i< fH*2; i++) {
		for (int j = 0; j<MAX_FRAME_LEN; j++) {
		SET_RCLK_H;
		*(rowBuf+i) = DATA_PINS;
		serialPtr->print(DATA_PINS, DEC);
    serialPtr->print("\t");
		SET_RCLK_L;

		}
    serialPtr->print("\n");
		//serialPtr->write(rowBuf, MAX_FRAME_LEN);
		//serialPtr->write(LF);
	}

}
