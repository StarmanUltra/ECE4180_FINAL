# Lightning Detection (ECE4180)
## Stefan Abi-Karam, Brian Harden, Larry Kresse
## Introduction
The idea behind this project is to create a personal device internet for outdoor recreation (ex. hiking, camping, ...) that alerts the user of lightning detected de to nearby storms. The personal device will mainly take advantage of the “SparkFun Lightning Detector - AS3935” coupled with a screen, small speaker, and Bluetooth-to-phone to alert the user when a strike is detected and approximately how far away it is. This device could also be connected to your mobile device through Bluetooth to also notify the user on their phone. We are also interested in logging the strikes detected by multiple devices,  either locally or by sending them to some database to be able to trilateral the exact location of all the lightning strikes detected. This can be useful for real-time alerts or analysis of a storm system.
## AS3935 Lightning Detector Module

![AS3935 Lightning Detection Module](https://github.com/StarmanUltra/ECE4180_FINAL/blob/main/images/as3935.jpg?raw=true)

The AS3935 Lightning Detector Module uses a 500 kHz resonance antenna to detect lightning and measure the strength. Using the strength measurements, the AS3935 module can calculate the distance of the lightning to the nearest kilometer (maximum of 40 kilometers away). The AS3935 can also be scaled down to detect sparks at distances in the centimeter range.
The AS3935 module utilizes an SPI interface. The pinout is like any other SPI device, except for one additional pin: INT. INT will be driven HIGH in the event that lightning is detected.
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

![Data Collection Station](https://github.com/StarmanUltra/ECE4180_FINAL/blob/main/images/data_collector_schematic.png?raw=true)

In this diagram, an external 5V power source will be needed, but the 3.3V input needed for the AS3935 module can be supplied by the mBed module. 
In order to communicate with the user's personal device, the personal device's IP address and an AP's credentials must be uploaded into each of the data collectors before powering on. It is highly recommended that the personal mbed device is booted first before the data collectors, though it does not matter which order they boot in.
Each station will have their own ID associated with it, so when it sends a message (as a struct), the personal mBed device will know which one sent which.
## The Personal Device
Here is a diagram showing the wiring diagram used for the personal mBed device the user will keep on him/her.

![Personal Device](https://github.com/StarmanUltra/ECE4180_FINAL/blob/main/images/personal_mbed_schematic.png?raw=true)

In this diagram, an external 5V power source will be needed, but the 3.3V input needed for the uLCD and SD card modules can be supplied by the mBed module. 
When booting up the device, it may be attached to a PC serial port for setting up, which will reveal its given IP address for the data collectors. The device itself acts as a TCP server, which will sleep until a TCP packet arrives. Upon arrival, it will loudly beep (hence the need for an amplifier for the speaker) and display information of the incoming data packet onto the LCD screen (such as lightning location). Typically, users will have earbuds in when they are hiking. This means we will need a bluetooth module to send a message to the phone using UART, which can interrupt the user's music. Alongside, it will also keep a record of lightning strikes into its SD card from each point.

