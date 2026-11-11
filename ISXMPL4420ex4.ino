// ISXMPL4420ex4 - Arduino I2C Current Loop Receiver Using the IS4420 Module
// https://inacks.com/is4420
//
// Reads the current, the percentage of the 4-20 mA span and the module
// temperature from the IS4420-M1 through the I2C-serial interface twice a
// second, and prints them on the Serial Monitor (115200 baud).
//
// Wiring (Arduino Uno):
//   IS4420-M1 3V3 -> Uno 3.3V
//   IS4420-M1 GND -> Uno GND and the loop power supply 0 V
//   IS4420-M1 ADR -> Uno GND                    (I2C address 26)
//   IS4420-M1 SPD -> Uno GND                    (100 kHz, the Wire default)
//   IS4420-M1 SDA -> level shifter -> Uno A4 (SDA)
//   IS4420-M1 SCL -> level shifter -> Uno A5 (SCL)
//   IS4420-M1 CIN -> current loop (the loop current returns through GND)
//   IS4420-M1 TX is not used in this example.
//
// The IS4420-M1 works at 3.3 V and the Uno's I2C pins at 5 V, so SDA and SCL
// go through a bidirectional I2C level shifter: its 3.3 V side to the
// IS4420-M1 and the Uno 3.3V pin, its 5 V side to the Uno and its 5V pin.
// Its resistors are the pull-ups the I2C-serial interface needs.
//
// The IS4420-M1 reads ADR and SPD at power-up: change them only with the
// module off.

#include <Wire.h>

#define IS4420_ADDRESS  26              // ADR tied to GND

void setup() {
  Serial.begin(115200);
  Wire.begin();
}

void loop() {
  uint8_t reg[7];

  // Every read starts by writing the 2-byte register address, high byte
  // first, here 0x0000. A repeated START then reads 7 registers, 0x0000 to
  // 0x0006, all from the same measurement:
  //   reg[0] MA_INT       reg[1] MA_FRAC        current in mA
  //   reg[2] PERCENT_INT  reg[3] PERCENT_FRAC   percentage of the span
  //   reg[4] MODULE_ID    reg[5] MODULE_REV     not used here
  //   reg[6] MODULE_TEMP                        module temperature
  Wire.beginTransmission(IS4420_ADDRESS);
  Wire.write(0x00);                             // register address, high byte
  Wire.write(0x00);                             // register address, low byte
  if (Wire.endTransmission(false) != 0 ||       // false: repeated START, no STOP
      Wire.requestFrom(IS4420_ADDRESS, 7) != 7) {
    // No answer. Once right after power-up is normal: the IS4420-M1
    // answers about 0.5 s after power-up.
    Serial.println("IS4420 not found");
    delay(500);
    return;
  }
  for (int i = 0; i < 7; i++) {
    reg[i] = Wire.read();
  }

  // Current and percentage
  if (reg[0] == 0xFE) {                         // FE 00 FE 00: no measurement
    Serial.print("No measurement");
  } else if (reg[0] == 0xFF) {                  // FF 00 FF 00: above 21.50 mA
    Serial.print("Over-range");
  } else {
    float current = reg[0] + reg[1] / 100.0;          // 12 and 34 -> 12.34 mA
    float percent = (reg[2] & 0x7F) + reg[3] / 10.0;  // 52 and 1  -> 52.1 %
    if (reg[2] & 0x80) percent = -percent;            // bit 7 set: below 4 mA
    Serial.print(current, 2);
    Serial.print(" mA   ");
    Serial.print(percent, 1);
    Serial.print(" %");
  }

  // Temperature: bit 7 = sign, bits 6-0 = degrees C. 0xFE = no measurement,
  // and then nothing is printed.
  if (reg[6] != 0xFE) {
    int temperature = reg[6] & 0x7F;
    if (reg[6] & 0x80) temperature = -temperature;
    Serial.print("   ");
    Serial.print(temperature);
    Serial.print(" degC");
  }
  Serial.println();

  delay(500);                           // twice a second; the module updates
                                        // its registers about every 100 ms
}
