#include <RF24.h>
#include <SPI.h>

// Instantiate an object for the nRF24L01 transceiver
RF24 radio(7, 8); // using pin 7 for the CE pin, and pin 8 for the CSN pin

uint8_t address[7] = "TXNode";

void setup(void) {
  Serial.begin(115200);
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
      ; // hold in infinite loop
  }
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
  radio.setPayloadSize(sizeof("65535, 65535, 65535"));
  radio.openReadingPipe(0, address);
  radio.startListening();
  Serial.flush();
}

void loop(void) {
  uint8_t pipe;
  if (radio.available(&pipe)) {
    Serial.println("Available");
    uint8_t bytes = radio.getPayloadSize();
    char text[bytes + 1];
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
}