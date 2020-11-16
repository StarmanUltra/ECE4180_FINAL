# Lightning Detection (ECE4180)
## Stefan Abi-Karam, Brian Harden, Larry Kresse
## Introduction
The idea behind this project is to create a personal device internet for outdoor recreation (ex. hiking, camping, ...) that alerts the user of lightning detected de to nearby storms. The personal device will mainly take advantage of the “SparkFun Lightning Detector - AS3935” coupled with a screen, small speaker, and Bluetooth-to-phone to alert the user when a strike is detected and approximately how far away it is. This device could also be connected to your mobile device through Bluetooth to also notify the user on their phone. We are also interested in logging the strikes detected by multiple devices,  either locally or by sending them to some database to be able to trilateral the exact location of all the lightning strikes detected. This can be useful for real-time alerts or analysis of a storm system.
## AS3935 Lightning Detector Module
The AS3935 Lightning Detector Module uses a 500 kHz resonance antenna to detect lightning and measure the strength. Using the strength measurements, the AS3935 module can calculate the distance of the lightning to the nearest kilometer (maximum of 40 kilometers away). The AS3935 can also be scaled down to detect sparks at distances in the centimeter range.
The AS3935 module utilizes an SPI interface, with the typical CS, SCK, MOSI, and MISO pins. The pinout is like any other SPI device, except for one additional pin: INT. INT will be driven high in the event that lightning is detected.
| Pin  | Function            |
|------|---------------------|
| MOSI | Master-Out-Slave-In |
| MISO | Master-In-Slave-Out |
| SCK  | Serial Clock        |
| CS   | Chip Select         |
| INT  | Interrupt In        |
| 3V3  | 3.3V Input          |
| GND  | Ground              |
## The Lightning Data Collection Stations
Here is a diagram showing the wiring diagram used for all three data collection stations

![Data Collection Station](https://github.com/StarmanUltra/ECE4180_FINAL/blob/main/data_collector_schematic.png?raw=true)

In this diagram, an external 5V power source will be needed, but the 3.3V input needed for the AS3935 module can be supplied by the mBed module. 
In order to communicate with the user's personal device, the personal device's IP address and an AP's credentials must be uploaded into each of the data collectors before powering on. It is highly recommended that the personal mbed device is booted first before the data collectors, though it does not matter which order they boot in.
Each station will have their own ID associated with it, so when it sends a message (as a struct), the personal mBed device will know which one sent which.
## The Personal Device
Here is a diagram showing the wiring diagram used for the personal mBed device the user will keep on him/her.
