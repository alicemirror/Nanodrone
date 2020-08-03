# Nanodrone
## The Nanodrone project evolution

The initial version of this project used a simple person recognition with Tensorflow Lite on **Arduino Nano Sense 33 BLE** initially created for the Nanorama Project14 event. You can find the original sources and some other stutt in the *Sense33* folder.

Starting from this initial prototype a new version has been started based on Raspberry Pi 4B, Arducan image acquisition and LoRa connection between the drone and the ground control with a couple of Arduino MKR 1300 (LoRa). The ground control uses a Cypress PSoC6 kit to post process the data and send them to the AWS IoT cloud.

### It is a work in progress
