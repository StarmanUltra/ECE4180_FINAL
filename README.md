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

## Lightning Locailzation Using True-range Multilateration (Work in progress)

![Trilateral Centroid Localization](https://github.com/StarmanUltra/ECE4180_FINAL/blob/main/images/trilateral_centroid_localization.png?raw=true)

The image shown above provides a visual on how the algorithm works once all three systems send in data.
***NEEDS MORE DETAIL

In order to localize the location of the detected stike, and thus the strom font, true-range multilateration is used. In true-range multilateration, the distances to the strike detected by each station at the same time can be used to caluclate the extact location of the strike. To localize a point in n-diminsion sapce, n+1 or more stations are needed. In this case, to localize the location of a stike in 2D, three stations are used.

Let us form some definitions that will be handy in solving the porblem:
- P_stike, (lat, lon) = stike location
- P_sn, (lat, lon), ex. P_s0 = detection station location
- d_n, km, ex. d_0 = distance to stike detected from station
- distance(P_a, P_b) = funcion to calculate the distance between two lat-lon points, haversince disance and equalangular distance are 2 fast and simple approximations

A nieve ideal apprach would fomulate the problem as follows:
Solve for P_strike (P_strike_lat and P_strike_lon) given the following constaints:
- d_0 = distance(P_strike, P_s0)
- d_1 = distance(P_strike, P_s1)
- d_2 = distance(P_strike, P_s2)

It may appear as if this is a system with 2 unkonwns and 3 equations. However, due to how the distances are calulcated, there can be an ambiguitywith the signs of certian values thus requireing the extra contraint.

If you tried to solve this system with real world data you will not find a solution since the distances will have some vairance and not overlap excatly to cre3ate a single solution. However, these ditance should overlap in a very very small region from which we can find the point that has the least error. Given this prespocitve, the poronelm can be tranlated into an optmization probolem tho find the point that solves the system with the smallest error.

The optimziation apprach is as follows:
- Reformulate the cointaints d_n = distance(P_strike, P_sn) as 0 = abs(distance(P_strike, P_sn) - d_n)
  - The right hand term of this equation, abs(distance(P_strike, P_sn) - d_n) give the absolute error of one staion's distance estimate for a given stike location
- Define E_n = abs(distance(P_strike, P_sn) - d_n) as the absolute error for a station distance prediction for a given stike location
- Define E = E_0 + E_1 + E_2 as  the sum of all absolute distance errors for a given stike location
  - Ideally, plugging in the the correct stike location should yeild E=0, however due to some vaiaince in the meausred distances E will be nonzero but small
 - Minimize E over P_strike (P_strike_lat and P_strike_lon)
