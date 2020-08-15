#include <LoRa.h>

String vals = "";

void setup() {
  Serial.begin(9600);
  //while (!Serial);
  Serial.println("LoRa Receiver");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
      vals= LoRa.readString() +"\nRSSI: " +LoRa.packetRssi(); 
      Serial.println(vals);
    }
  }
  
}
