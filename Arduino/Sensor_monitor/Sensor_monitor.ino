#include <Adafruit_TCS34725.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>

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

uint8_t address[][6] = {"1Node", "2Node"}; // Let these addresses be used for
                                           // the pair
bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to

bool role = true; // true = TX role, false = RX role

void getRawData_noDelay(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {
  *c = tcs.read16(TCS34725_CDATAL);
  *r = tcs.read16(TCS34725_RDATAL);
  *g = tcs.read16(TCS34725_GDATAL);
  *b = tcs.read16(TCS34725_BDATAL);
}

void setup(void) {
  pinMode(interruptPin, INPUT_PULLUP); // TCS interrupt output is Active-LOW
                                       // and Open-Drain
  attachInterrupt(digitalPinToInterrupt(interruptPin), isr, FALLING);

  Serial.begin(115200);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ; // hold in infinite loop
  }

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
      ; // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
  radio.setPayloadSize(sizeof("255, 255, 255"));
  radio.openWritingPipe(address[0]);
  radio.stopListening();

  // Set persistence filter to generate an interrupt for every RGB Cycle,
  // regardless of the integration limits
  tcs.write8(TCS34725_PERS, TCS34725_PERS_NONE);
  tcs.setInterrupt(true);

  Serial.flush();
}

void loop(void) {
  uint16_t r, g, b, c;
  if (state) {
    getRawData_noDelay(&r, &g, &b, &c);
    tcs.clearInterrupt();
    state = false;
  }

  char payload[20] = "";
  sprintf(payload, "%d, %d, %d", r, g, b);
  radio.write(&payload, sizeof(payload));

  Serial.println(payload);

  delay(1000);
}
