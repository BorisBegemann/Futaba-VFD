
#include "FVFD.h"
#include "splash.h"
#include <Adafruit_GFX.h>
#include <pin_configuration.h>

volatile bool FVFD::waitingToSend = false;
volatile bool FVFD::clearedToSend = false;

FVFD::FVFD() : Adafruit_GFX(1024, 64)
{
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, 1);
  pinMode(UNK0_PIN, OUTPUT);
  digitalWrite(UNK0_PIN, 1);
  pinMode(UNK1_PIN, OUTPUT);
  digitalWrite(UNK1_PIN, 1);
  pinMode(UNK2_PIN, OUTPUT);
  digitalWrite(UNK2_PIN, 1);

  pinMode(CD_PIN, OUTPUT);
  digitalWrite(CD_PIN, 0);
  pinMode(WR_PIN, OUTPUT);
  digitalWrite(WR_PIN, 1);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, 1);

  pinMode(D0_PIN, OUTPUT);
  pinMode(D1_PIN, OUTPUT);
  pinMode(D2_PIN, OUTPUT);
  pinMode(D3_PIN, OUTPUT);
  pinMode(D4_PIN, OUTPUT);
  pinMode(D5_PIN, OUTPUT);
  pinMode(D6_PIN, OUTPUT);
  pinMode(D7_PIN, OUTPUT);

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), handleInt, RISING);
}

FVFD::~FVFD(void) {
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
  detachInterrupt(digitalPinToInterrupt(INT_PIN));
}

bool FVFD::begin() 
{
  if ((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * (HEIGHT / 8))))
    return false;

  waitingToSend = false;
  clearedToSend = false;

  clearDisplay();
  drawBitmap((336 - splash_width) / 2, (24 - splash_height) / 2,
            splash_data, splash_width, splash_height, 1);

  sendCommand(CMD_CONTROL_POWER);
  sendData(0);
  sendCommand(CMD_DISPLAY_MODE);
  sendData(16);
  sendCommand(CMD_CLEAR_SCREEN);
  sendCommand(CMD_SET_FIRST_DISPLAY_ADDR_LO);
  sendData(0);
  sendCommand(CMD_SET_FIRST_DISPLAY_ADDR_HI);
  sendData(0);
  sendCommand(CMD_FIRST_DISPLAY_ON);
  sendCommand(CMD_AUTO_INCREMENT);
  sendCommand(CMD_SET_READWRITE_ADDR_LO);
  sendData(0);
  sendCommand(CMD_SET_READWRITE_ADDR_HI);
  sendData(0);

  display();
  return true;
}

void FVFD::handleInt(void)
{
  if(waitingToSend){
    clearedToSend = true;
    waitingToSend = false;
  }
}

void FVFD::waitToSend(void)
{
  clearedToSend = false;
  waitingToSend = true;
  while (!clearedToSend);
}

void FVFD::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    switch (color) {
    case FVFD_WHITE:
      buffer[x*8+(y/8)] |= (0b10000000 >> (y % 8));
      break;
    case FVFD_BLACK:
      buffer[x*8+(y/8)] &= ~(0b10000000 >> (y % 8));
      break;
    case FVFD_INVERSE:
      buffer[x*8+(y/8)] ^= (0b10000000 >> (y % 8));
      break;
    }
  }
}

void FVFD::clearDisplay(void) {
  memset(buffer, 0, WIDTH * (HEIGHT / 8));
}

bool FVFD::getPixel(int16_t x, int16_t y) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    return (buffer[x*8+(y/8)] & (1 << (y % 8)));
  }
  return false;
}

uint8_t *FVFD::getBuffer(void) { return buffer; }

void FVFD::setDisplayOrigin(int orig_x, int orig_y){
  if ((orig_x >= 0) && (orig_x < width()) && (orig_y >= 0) && (orig_y < height())) {
    this->waitToSend();
    u_int16_t index = orig_x * 8 + orig_y;
    sendCommand(CMD_SET_FIRST_DISPLAY_ADDR_LO);
    sendData(index & 0xff);
    sendCommand(CMD_SET_FIRST_DISPLAY_ADDR_HI);
    sendData((index>>8) & 0xff);
    sendCommand(CMD_FIRST_DISPLAY_ON);
  }
}

void FVFD::display(void) {
  this->waitToSend();
  sendCommand(CMD_SET_READWRITE_ADDR_LO);
  sendData(0);
  sendCommand(CMD_SET_READWRITE_ADDR_HI);
  sendData(0);
  sendCommand(CMD_SEND_DATA);

  for(int x = 0; x < (width() * (height() / 8))-1; x++)
  {
      sendData(buffer[x]);
  }
}

void FVFD::busWrite(char data)
{
    digitalWrite(CS_PIN, 0);
    digitalWrite(WR_PIN, 0);

    digitalWrite(D0_PIN, (data >> 0) & 1);
    digitalWrite(D1_PIN, (data >> 1) & 1);
    digitalWrite(D2_PIN, (data >> 2) & 1);
    digitalWrite(D3_PIN, (data >> 3) & 1);
    digitalWrite(D4_PIN, (data >> 4) & 1);
    digitalWrite(D5_PIN, (data >> 5) & 1);
    digitalWrite(D6_PIN, (data >> 6) & 1);
    digitalWrite(D7_PIN, (data >> 7) & 1);

    digitalWrite(WR_PIN, 1);
    digitalWrite(CS_PIN, 1);
}

void FVFD::sendCommand(char cmd)
{
    digitalWrite(CD_PIN, 1);
    busWrite(cmd);
    digitalWrite(CD_PIN, 0);
}

void FVFD::sendData(char data)
{
    digitalWrite(CD_PIN, 0);
    busWrite(data);
}
