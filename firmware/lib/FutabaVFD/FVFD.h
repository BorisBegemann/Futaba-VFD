#ifndef _FVFD_H_
#define _FVFD_H_

#define FVFD_NO_SPLASH

#include <Adafruit_GFX.h>

#define FVFD_BLACK 0
#define FVFD_WHITE 1
#define FVFD_INVERSE 2

#define FVFD_MEMORYMODE 0x20          ///< See datasheet

#define CMD_BOTH_DISPLAYS_OFF           0x00
#define CMD_FIRST_DISPLAY_ON            0x01
#define CMD_SECOND_DESPLAY_ON           0x02
#define CMD_AUTO_INCREMENT              0x04
#define CMD_NO_INCREMENT                0x05
#define CMD_CLEAR_SCREEN                0x06
#define CMD_CONTROL_POWER               0x07
#define CMD_SEND_DATA                   0x08
#define CMD_READ_DATA                   0x09
#define CMD_SET_FIRST_DISPLAY_ADDR_LO   0x0A
#define CMD_SET_FIRST_DISPLAY_ADDR_HI   0x0B
#define CMD_SET_SECOND_DISPLAY_ADDR_LO  0x0C
#define CMD_SET_SECOND_DISPLAY_ADDR_HI  0x0D
#define CMD_SET_READWRITE_ADDR_LO       0x0E
#define CMD_SET_READWRITE_ADDR_HI       0x0F
#define CMD_LUMINANCE                   0x13
#define CMD_DISPLAY_MODE                0x14
#define CMD_INT_SIGNAL_MODE             0x15

class FVFD : public Adafruit_GFX {
public:
  FVFD();

  ~FVFD(void);

  bool begin();
  void display(void);
  void setDisplayOrigin(int orig_x, int orig_y);
  void clearDisplay(void);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  bool getPixel(int16_t x, int16_t y);
  uint8_t *getBuffer(void);

  void busWrite(char data);
  void sendCommand(char cmd);
  void sendData(char data);

  static volatile bool waitingToSend;
  static volatile bool clearedToSend;

private:
  static void handleInt(void);
  static void waitToSend(void);
  
protected:
  uint8_t *buffer;  
};

#endif