#include <SPI.h>
#include <LoRa.h>
#include <Streaming.h>

int counter = 0;
String val="";

void setup() {
 if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  LoRa.beginPacket();
  LoRa << "Nanodrone test " << counter << endl;
  LoRa.endPacket();
  counter++; // this help keep track if the packet is recived on the sender side

  delay(5000);
}
