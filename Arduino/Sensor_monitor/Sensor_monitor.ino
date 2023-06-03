#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "RF24.h"

#define RED 9
#define GREEN 10
#define BLUE 11

#define IN_MIN 0
#define IN_MAX 4095
#define OUT_MIN 1
#define OUT_MAX 255

int valueR, valueG, valueB;

// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
const int interruptPin = 2;
volatile boolean state = false;

//Interrupt Service Routine
void isr() {
  state = true;
}

/* tcs.getRawData() does a delay(Integration_Time) after the sensor readout.
We don't need to wait for the next integration cycle
because we receive an interrupt when the integration cycle is complete */
void getRawData_noDelay(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {
  *c = tcs.read16(TCS34725_CDATAL);
  *r = tcs.read16(TCS34725_RDATAL);
  *g = tcs.read16(TCS34725_GDATAL);
  *b = tcs.read16(TCS34725_BDATAL);
}

void setup(void) {
  valueR = 0;
  valueG = 0;
  valueB = 0;

  pinMode(interruptPin, INPUT_PULLUP);  //TCS interrupt output is Active-LOW and Open-Drain
  attachInterrupt(digitalPinToInterrupt(interruptPin), isr, FALLING);

  Serial.begin(9600);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ;
  }

  // Set persistence filter to generate an interrupt for every RGB Cycle,
  // regardless of the integration limits
  tcs.write8(TCS34725_PERS, TCS34725_PERS_NONE);
  tcs.setInterrupt(true);

  Serial.flush();

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;

  if (state) {
    getRawData_noDelay(&r, &g, &b, &c);
    // colorTemp = tcs.calculateColorTemperature(r, g, b);
    colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
    lux = tcs.calculateLux(r, g, b);
    tcs.clearInterrupt();
    state = false;
  }

  // valueR = (valueR + (r == 0 ? 0 : map(r, 0, 65535, 1, 256))) / 2;
  // valueG = (valueG + (g == 0 ? 0 : map(g, 0, 65535, 1, 256))) / 2;
  // valueB = (valueB + (b == 0 ? 0 : map(b, 0, 65535, 1, 256))) / 2;

  if (valueR > (r == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    valueR--;
  } else if (valueR == (r == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    ;
  } else {
    valueR++;
  }
  if (valueG > (g == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    valueG--;
  } else if (valueG == (g == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    ;
  } else {
    valueG++;
  }
  if (valueB > (b == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    valueB--;
  } else if (valueB == (b == 0 ? 0 : map(r, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX))) {
    ;
  } else {
    valueB++;
  }

  valueR = constrain(valueR, 0, 255);
  valueG = constrain(valueG, 0, 255);
  valueB = constrain(valueB, 0, 255);

  analogWrite(RED, valueR);
  analogWrite(GREEN, valueG);
  analogWrite(BLUE, valueB);

  // Serial.print("RED:");
  // Serial.print(valueR);
  // Serial.print(",");
  // Serial.print("GREEN:");
  // Serial.print(valueG);
  // Serial.print(",");
  // Serial.print("BLUE:");
  // Serial.println(valueB);

  // analogWrite(RED, r == 0 ? 0 : map(r, 0, 65535, 1, 256));
  // analogWrite(GREEN, g == 0 ? 0 : map(g, 0, 65535, 1, 256));
  // analogWrite(BLUE, b == 0 ? 0 : map(b, 0, 65535, 1, 256));

  Serial.print("Color Temp: ");
  Serial.print(colorTemp, DEC);
  Serial.print(" K - ");
  Serial.print("Lux: ");
  Serial.print(lux, DEC);
  Serial.print(" - ");
  Serial.print("R: ");
  Serial.print(valueR, DEC);
  Serial.print(" ");
  Serial.print("G: ");
  Serial.print(valueG, DEC);
  Serial.print(" ");
  Serial.print("B: ");
  Serial.print(valueB, DEC);
  Serial.print(" ");
  Serial.print("C: ");
  Serial.print(c, DEC);
  Serial.print(" ");
  Serial.println(" ");

  delay(10);
}
