#include <Arduino.h>
#include <Bounce.h>
#include <SPI.h>

/**
 * Hardware setup:
 * 3x 595 shift registers connected to SPI0
 * shift registers outputs connected to LEDs
 */

// PINS
const int latchPin = 10;
const int player1Btn = 0;
const int player2Btn = 1;
Bounce player1 = Bounce(player1Btn, 10);
Bounce player2 = Bounce(player2Btn, 10);

const int wordSize = 24;
constexpr int bytes = wordSize / 8;
constexpr uint32_t resetThreshold = 1UL << wordSize;
constexpr uint32_t maxValue = resetThreshold >> 1;
// initialize game value to the middle
uint32_t value = 1UL << (wordSize >> 1);

SPISettings settings(2000000, MSBFIRST, SPI_MODE0);

void writeSPI(uint32_t value);

void setup()
{
  pinMode(player1Btn, INPUT_PULLUP);
  pinMode(player2Btn, INPUT_PULLUP);
  pinMode(latchPin, OUTPUT);
  digitalWrite(latchPin, LOW);
  SPI.begin();
  // make sure all the LEDs are working
  for (uint32_t i = 1; i < resetThreshold; i <<= 1)
  {
    writeSPI(i);
    delay(100);
  }
  writeSPI(value);
}

void loop()
{
  bool changed = false;
  if (player1.update())
  {
    if (player1.fallingEdge())
    {
      if (value > 1)
      {
        value >>= 1;
        changed = true;
      }
    }
  }

  if (player2.update())
  {
    if (player2.fallingEdge())
    {
      if (value < maxValue)
      {
        value <<= 1;
        changed = true;
      }
    }
  }

  if (changed)
    writeSPI(value);
}

void writeSPI(uint32_t value)
{
  SPI.beginTransaction(settings);
  digitalWrite(latchPin, LOW);
  for (int i = bytes - 1; i >= 0; --i)
  {
    SPI.transfer((value >> 8 * i) & 0xFF);
  }
  digitalWrite(latchPin, HIGH);
  SPI.endTransaction();
}
