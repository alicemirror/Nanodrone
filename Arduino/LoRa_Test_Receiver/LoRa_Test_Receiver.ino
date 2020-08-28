#include <LoRa.h>
#include <Streaming.h>

#define SYS_LED 6

void setup() {
  // Initialize the monitoring serial
  Serial.begin(38400);
  // Initialize the communication serial
  Serial1.begin(115200);

  // Notify the LoRa activity
  pinMode(SYS_LED, OUTPUT);
  digitalWrite(SYS_LED, LOW);

  Serial << "LoRa Receiver" << endl;

  if (!LoRa.begin(915E6)) {
    Serial <<"Starting LoRa failed! System halted." << endl;
    while (1);
  }
}


void loop() {
  int packetSize = LoRa.parsePacket();

  if(Serial1.available()) {
    Serial << "From PSoC6 " << Serial1.readString() << endl;
  }
  
  if (packetSize) {
    digitalWrite(SYS_LED, HIGH);
    Serial << "Receiving LoRa packet of " << packetSize << " bytes" << endl;
    // read packet
    while (LoRa.available()) {
      String inPacket = LoRa.readString();
      Serial << "Sending >>" << inPacket << endl;
      Serial1.println(inPacket); 
    }
    digitalWrite(SYS_LED, LOW);
  }
  
}
