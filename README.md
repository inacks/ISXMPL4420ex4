# Arduino I2C Current Loop Receiver Using the IS4420 Module

<p align="center">
  <a href="https://inacks.com/product/4-20ma-receiver-is4420-m1/">
    <img src="https://inacks.com/wp-content/uploads/IS4420-M1-I2C-UART-Current-Loop-Receiver-1.jpg" alt="IS4420-M1 I2C/UART 4-20 mA current loop receiver module" width="250">
  </a>
</p>

This example is powered by the **IS4420-M1**, an I2C/UART 4-20 mA current loop receiver module that lets any microcontroller with I2C read a current loop with a single I2C read.

No analog circuitry to design. No level shifter: the I2C pads of the IS4420-M1 are 5 V tolerant. No extra libraries. No calibration needed: INACKS calibrates every module at 4 mA and 20 mA.  
The IS4420-M1 contains the complete measurement side of the receiver: the loop current flows into its CIN pad, and your code reads the current in mA, the percentage of the 4-20 mA span and the module temperature from its registers.

Perfect for reading pressure, temperature, flow, level and weight transmitters from an Arduino, for adding 4-20 mA inputs to your own controllers and data loggers, and for reading up to five current loops on the same I2C-serial interface.

### 🛒 [Buy the IS4420-M1 and learn more](https://inacks.com/product/4-20ma-receiver-is4420-m1/)

---

## What this example does

- Reads the current, the percentage of the 4-20 mA span and the module temperature twice a second.
- Prints them on the Serial Monitor at 115200 baud.
- Prints `Over-range` when the current is above 21.50 mA, and `No measurement` when the module has no valid measurement.
- Prints `IS4420 not found` if the IS4420-M1 does not acknowledge the I2C transfer.

To read the module through its UART output instead, see [ISXMPL4420ex3](https://github.com/inacks/ISXMPL4420ex3).

---

## What you need

- An [IS4420-M1 module](https://inacks.com/product/4-20ma-receiver-is4420-m1/)
- An Arduino Uno
- Two 4.7 kΩ resistors, the pull-ups for SDA and SCL
- A 4-20 mA transmitter (2-wire or 3-wire) and its loop power supply, or a 4-20 mA current source
- The Arduino IDE. The sketch only uses the standard `Wire` library, which comes with the IDE.

---

## Wiring

| IS4420-M1 pad | Connect to |
|---|---|
| 3V3 | Arduino 3.3V |
| GND | Arduino GND and the loop power supply 0 V |
| SDA | Arduino SDA (A4), and a 4.7 kΩ pull-up to the 5V pin |
| SCL | Arduino SCL (A5), and a 4.7 kΩ pull-up to the 5V pin |
| ADR | GND: I2C address 26 |
| SPD | GND: 100 kHz |
| CIN | Current loop input: the loop current flows into CIN and returns through GND |
| TX | Leave it floating, with nothing connected. Never connect it to GND: the module doesn't start while TX is held low. |

- The IS4420-M1 takes its 3.3 V supply from the Arduino 3.3V pin.
- The SDA and SCL pads are 5 V tolerant, so they connect straight to the Uno's I2C pins, with no level shifter. The IS4420-M1 has no pull-ups of its own: the two 4.7 kΩ resistors to the 5V pin are the pull-ups the I2C-serial interface needs.
- With the pull-ups at 5 V, keep the IS4420-M1 powered whenever that 5 V is present: unpowered, its pads accept 4 V at most. Here the Uno powers both, so they're always on together.
- With a 3.3 V board, pull SDA and SCL up to 3.3 V instead.
- On other Arduino boards, use the board's SDA and SCL pins, for example pins 20 and 21 on the Arduino Mega 2560.
- With a 2-wire transmitter: loop power supply + to the transmitter +, the transmitter - to CIN, and the loop power supply 0 V to GND.
- With a 3-wire transmitter: the transmitter output to CIN, and the transmitter 0 V and the loop power supply 0 V to GND.
- The current loop input isn't isolated: the loop power supply 0 V, the IS4420-M1 GND and the Arduino GND are connected together.

---

## Running the example

1. Wire the circuit.
2. Open the sketch in the Arduino IDE.
3. Select your board and port, and upload the sketch.
4. Open the Serial Monitor at 115200 baud.

You should see a new line twice a second. With 12 mA in the loop:

```
12.00 mA   50.0 %   26 degC
12.00 mA   50.0 %   26 degC
12.00 mA   50.0 %   26 degC
...
```

The percentage is the position of the current in the 4-20 mA span:

| Loop current | Percentage |
|:---:|:---:|
| Below 4 mA | Negative |
| 4 mA | 0.0 % |
| 12 mA | 50.0 % |
| 20 mA | 100.0 % |
| 21.50 mA | 109.4 % |
| Above 21.50 mA | `Over-range` |

---

## How it works

The Arduino (I2C master) reads the measurement with a single I2C transfer. It writes the 2-byte register address 0x0000 and, after a repeated START, reads 7 registers:

```
START | 26+W | 0x00 | 0x00 | RESTART | 26+R | 7 data bytes | STOP
```

| Address | Register | Content |
|:---:|---|---|
| 0x0000 | MA_INT | Current, whole mA |
| 0x0001 | MA_FRAC | Current, hundredths of mA |
| 0x0002 | PERCENT_INT | Percentage, whole %. Bit 7 is the sign: set below 4 mA. |
| 0x0003 | PERCENT_FRAC | Percentage, tenths of % |
| 0x0004 | MODULE_ID | Module identifier (not used here) |
| 0x0005 | MODULE_REV | Module revision (not used here) |
| 0x0006 | MODULE_TEMP | Module temperature, whole °C. Bit 7 is the sign: set below 0 °C. 254 (0xFE) means no measurement. |

Some examples of the first four registers:

| Bytes read | Meaning |
|:---:|---|
| 12, 34, 52, 1 | 12.34 mA, 52.1 % |
| 2, 0, 140, 5 | 2.00 mA, -12.5 % (bit 7 of 140 is set, and 140 - 128 = 12) |
| 255, 0, 255, 0 (0xFF 00 FF 00) | Over-range: the current is above 21.50 mA |
| 254, 0, 254, 0 (0xFE 00 FE 00) | No measurement |

In the sketch, the read and the conversion are just this:

```cpp
Wire.beginTransmission(IS4420_ADDRESS);
Wire.write(0x00);                                  // register address 0x0000, high byte
Wire.write(0x00);                                  // register address 0x0000, low byte
Wire.endTransmission(false);                       // repeated START, no STOP
Wire.requestFrom(IS4420_ADDRESS, 7);               // registers 0x0000 to 0x0006
for (int i = 0; i < 7; i++) reg[i] = Wire.read();

float current = reg[0] + reg[1] / 100.0;           // 12 and 34 -> 12.34 mA
float percent = (reg[2] & 0x7F) + reg[3] / 10.0;   // 52 and 1  -> 52.1 %
if (reg[2] & 0x80) percent = -percent;             // bit 7 set: below 4 mA
```

The same transfer works on any microcontroller with an I2C master interface, such as an STM32 or an ESP32.

---

## Notes

- **Register address first:** every read starts by writing the 2-byte register address. Reads without it are not supported.
- **Consistent readings:** each read returns a copy of the registers taken when the read starts, so the current, the percentage and the temperature always come from the same measurement.
- **Update rate:** the IS4420-M1 updates its registers once per measurement, about every 100 ms. The sketch reads them twice a second.
- **Clock stretching:** the IS4420-M1 holds SCL low for a few milliseconds while it prepares the data. The Arduino `Wire` library handles it.
- **Several modules:** ADR selects the I2C address, so up to five IS4420-M1 modules can share the same I2C-serial interface. SPD selects the speed. The module reads both pads at power-up, so only change them with the module off. Then update `IS4420_ADDRESS`, and call `Wire.setClock()` to match the speed. Faster speeds need stronger pull-ups: 2 kΩ at 400 kHz and 1 kΩ at 1 MHz.

  | ADR | I2C address |
  |:---:|:---:|
  | GND | 26 |
  | 1/4 of 3V3 | 27 |
  | 1/2 of 3V3 | 28 |
  | 3/4 of 3V3 | 29 |
  | 3V3 | 30 |

  | SPD | Speed |
  |:---:|:---:|
  | GND | 100 kHz |
  | 1/2 of 3V3 | 400 kHz |
  | 3V3 | 1 MHz |

  For the intermediate levels, use a resistor divider between 3V3 and GND: two 10 kΩ resistors for 1/2 of 3V3, or 30 kΩ and 10 kΩ for 1/4 and 3/4 of 3V3 (the 10 kΩ resistor goes to GND for 1/4 and to 3V3 for 3/4).
- **Temperature:** it's the temperature inside the module, which can be above the ambient temperature.
- **Protection:** CIN has no protection. Never connect the loop power supply straight to CIN, and never reverse the loop: both destroy the module.

---

## Troubleshooting

If the Serial Monitor shows `IS4420 not found`, the IS4420-M1 did not acknowledge the I2C transfer. One such line right after power-up is normal: the module answers about 0.5 s after power-up. If it keeps showing, check:

- SDA on A4 and SCL on A5, and the 4.7 kΩ pull-ups to the 5V pin.
- The 3V3 and GND connections.
- ADR and SPD, both connected to GND in this example. The module reads them at power-up, so power-cycle it after changing them.

| Serial Monitor shows | What to check |
|---|---|
| About `0.00 mA   -25.0 %` | There's no loop current. Check the CIN connection, the transmitter and its loop power supply, and that the loop power supply 0 V goes to GND. |
| `Over-range` | The loop current is above 21.50 mA. Check the transmitter. |
| `No measurement` | Normal for a moment right after power-up, before the first measurement. If it keeps showing, check the 3V3 supply (3.3 V). |

---

## Get the IS4420-M1

- 🛒 **[Buy the IS4420-M1](https://inacks.com/product/4-20ma-receiver-is4420-m1/)**: I2C/UART 4-20 mA Current Loop Receiver Module
- 📄 [IS4420-M1 datasheet (ISDOC146)](https://inacks.com/wp-content/uploads/IS4420-M1-Datasheet-I2C-4-20mA-Current-Loop-Receiver-ISDOC146.pdf): electrical specification, pad description, UART line format, I2C memory map and hardware examples

ℹ️ For more information: [www.inacks.com](https://www.inacks.com)
