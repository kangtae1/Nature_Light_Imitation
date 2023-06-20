#include <RF24.h>
#include <SPI.h>

#define DATA_SIZE 32

// Instantiate an object for the nRF24L01 transceiver
RF24 radio(7, 8); // using pin 7 for the CE pin, and pin 8 for the CSN pin

uint8_t address[7] = "TXNode";

void setup(void) {
  Serial.begin(115200);

  // wait until radio.begin() returns true
  while (!radio.begin()) {
    Serial.println("No radio module found");
    delay(1000);
  }
  Serial.println("Found radio module");

  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MAX is default.
  radio.setPayloadSize(DATA_SIZE);
  radio.openReadingPipe(0, address);
  radio.startListening();
  Serial.flush();
}

void loop(void) {
  uint8_t pipe;
  if (radio.available(&pipe)) {
    uint8_t bytes = radio.getPayloadSize();
    char text[bytes + 1];
    radio.read(&text, sizeof(text));

    uint16_t r, g, b, c;
    // parse the text
    sscanf(text, "%u, %u, %u, %u", &r, &g, &b, &c);
    // normalize the values to 0-255 range but round values
    r = (r == 0 ? 0 : map(r, 1, 16383, 1, 255));
    g = (g == 0 ? 0 : map(g, 1, 16383, 1, 255));
    b = (b == 0 ? 0 : map(b, 1, 16383, 1, 255));

    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    // print the values
    Serial.print("R: ");
    Serial.print(r);
    Serial.print(" G: ");
    Serial.print(g);
    Serial.print(" B: ");
    Serial.println(b);
    Serial.println(text);
  }
}