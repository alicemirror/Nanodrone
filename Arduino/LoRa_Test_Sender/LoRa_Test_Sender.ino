#include <SPI.h>
#include <LoRa.h>

int counter = 0;
String val="";

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("LoRa Sender");

 if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {

  Serial.println("------------------------------------");
  Serial.println();
  Serial.print("Sending packet: ");
  Serial.println(counter);
  LoRa.beginPacket();
  LoRa.print("Test");
  LoRa.endPacket();
  counter++; // this help keep track if the packet is recived on the sender side

  delay(5000);
}
