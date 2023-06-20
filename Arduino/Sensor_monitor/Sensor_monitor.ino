#include <Adafruit_TCS34725.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>

#define DATA_SIZE 32

int valueR, valueG, valueB;

// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs =
    Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
const int interruptPin = 2;
volatile boolean state = false;

// Interrupt Service Routine
void isr() { state = true; }

// Instantiate an object for the nRF24L01 transceiver
RF24 radio(7, 8); // using pin 7 for the CE pin, and pin 8 for the CSN pin

uint8_t address[7] = "TXNode";

void getRawData_noDelay(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {
  *c = tcs.read16(TCS34725_CDATAL);
  *r = tcs.read16(TCS34725_RDATAL);
  *g = tcs.read16(TCS34725_GDATAL);
  *b = tcs.read16(TCS34725_BDATAL);
}

void setup(void) {
  pinMode(interruptPin,
          INPUT_PULLUP); // TCS interrupt output is Active-LOW and Open-Drain
  attachInterrupt(digitalPinToInterrupt(interruptPin), isr, FALLING);

  // Serial.begin(115200);

  // wait until tcs.begin() returns true
  while (!tcs.begin()) {
    // Serial.println("No TCS34725 found");
    delay(1000);
  }
  // Serial.println("Found TCS34725 sensor");

  // wait until radio.begin() returns true
  while (!radio.begin()) {
    // Serial.println("No radio module found");
    delay(1000);
  }
  // Serial.println("Found radio module");

  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MAX is default.
  radio.setPayloadSize(DATA_SIZE);
  radio.openWritingPipe(address);
  radio.stopListening();

  // Set persistence filter to generate an interrupt for every RGB Cycle,
  // regardless of the integration limits
  tcs.write8(TCS34725_PERS, TCS34725_PERS_NONE);
  tcs.setInterrupt(true);

  // Serial.flush();
}

void loop(void) {
  // if interrupt is triggered, read the data and send raw data to the receiver
  if (state) {
    uint16_t r, g, b, c;
    getRawData_noDelay(&r, &g, &b, &c);

    // send raw data to the receiver
    char payload[DATA_SIZE];
    sprintf(payload, "%u, %u, %u, %u", r, g, b, c);
    radio.write(&payload, sizeof(payload));
    // Serial.println(payload);

    state = false;
    // clear interrupt flag
    tcs.clearInterrupt();
  }
}
