/*
Authors: Brian Harden + Stefan Abi-Karam + Larry Kresse
Class: ECE4180 - Fall 2020
Description: The purpose of this program is to set up a single mbed device

BRIAN'S mBED MAC ADDRESS:   00:02:f7:f3:64:9f
BORROWED mBED MAC ADDRESS:  00:02:F7:F3:D2:C5
MAIN ESP8266 IP ADDRESS:    143.215.99.29
BORROWED mBED IP ADDRESS:   143.215.104.115
MAIN ESP8266 MAC ADDRESS:   18-FE-34-05-99-F8
BORROW ESP8266 MAC ADDRESS: 18-FE-34-0D-CB-40

*/


#include <string>
#include "mbed.h"
#include "ESP8266.h"
#include "AS3935.h"

#define LIGHTNINGDETECTORID 1

//DECLARATIONS: STRUCTS
// Struct to send over TCP
struct DATA
{
    char detectorID;
    char distanceKM;
    unsigned long time;
};

union rawReceivedData
{
    struct DATA struc;
    char dataString[sizeof(DATA)];
};

//using namespace std::chrono
//DECLARATIONS: GLOBAL VARIABLES
DigitalOut led1(LED1);                                                          //DEBUGGING: On-board LED used for debugging purposes
DigitalOut led2(LED2);                                                          //DEBUGGING: On-board LED used for debugging purposes
DigitalOut led4(LED4);                                                          //DEBUGGING: On-board LED used for debugging purposes
char dataTEMP[8] = "5";                                                         //DEBUGGING: A buffer to store the incoming data
Serial pc(USBTX, USBRX);                                                        //Set up the mbed USB port for debugging/monitoring
Timer clocky;                                                                    //A clock used to log the time of each lightning strike
ESP8266 wifi(p28, p27, p26, 9600, 3000);                                        //The WiFi module
AS3935 ld(p11, p12, p13, p14, "ld", 2000000);                                   //MOSI, MISO, SCK, CS, SPI bus freq (hz)
InterruptIn as3935INT(p15);                                                     //Interrupt signal that is given by the AS3935 Lightning Detector
DigitalOut wifiRST(p26);                                                        //Reset signal to the ESP8266 Device
char ssid[32] = "GTother";                                                      //enter WiFi router ssid inside the quotes
char pwd [32] = "GeorgeP@1927";                                                 //enter WiFi router password inside the quotes
char serverIP [32] = "143.215.99.29";                                           //The server IP to connect to (the main mBed)
//char serverIP [32] = "143.215.104.115";                                       //The server IP to connect to (the main mBed)
Timer timeoutTimer;                                                             //A timer in case of ESP8266 timeouts
unsigned int timeout;                                                           //A maximum time (in seconds) before timeout
int county;                                                                     //A counter for timeouts in the ESP8266
bool ended;                                                                     //A boolean letting us know when the ESP8266's role is terminated
union rawReceivedData dataToSend;                                                  //A struct to send over TCP to the server


//DECLARATIONS: FUNCTION PROTOTYPES
void LightningDetected();                                                       //Interrupt routine to handle the event of lightning occurring
void SetupTransmitter();                                                        //Sets up the WiFi card for transmitting
void SetupLightningDetector();                                                  //Sets up the AS3935 lightning detector
void dev_recv();                                                                //DEBUGGING: Write out any errors that may occur within the WiFi module
void pc_recv();                                                                 //DEBUGGING: Write out any errors that may occur within the WiFi module

/*******************************************************************************
MAIN FUNCTION
*******************************************************************************/

int main() 
{
    wifiRST = 0;                                                                //Reset the ESP8266
    wait(0.5);                                                                  //Give it a little time to fully reset
    wifiRST = 1;                                                                //Raise the reset pin
    wait(1);                                                                    //Give it a second to re-initialize
    //Set up the transmitter
    SetupTransmitter();
    //Initialize the lightning detector
    SetupLightningDetector();
    pc.printf("Ready!\r\n");                                                    //DEBUGGING: Let the debugger know it's ready
    clocky.start();                                                             //Start the clock
    while(1) 
    {
        //Do nothing in the main thread
        wait(1);
    }
}

/**************************
LIGHTNING DETECTED
***************************/
//Summary: The AS3935 breakout board will send the INT signal high when it
// detects lightning. This interrupt function is designed to handle the
// interrupt in the event of lightning being detected
void LightningDetected()
{
    clocky.stop();                                                              //Stop the clock to record the time
    led1 = 1;                                                                   //DEBUGGING: Blink the LED
    char OriginInt;                                                             //DEBUGGING: Declare variable for obtaining the debug code
    wait_ms(2);
    OriginInt = ld.interruptSource();                                           //Gather the debug value
    if (OriginInt == 1) 
    { //
        pc.printf(" Noise level too high\r\n");
    }
    if (OriginInt == 4) 
    { //
        pc.printf(" Disturber\r\n");
    }
    if (OriginInt == 8) 
    { // detection
        dataTEMP[0] = ld.lightningDistanceKm();                                 //Shove the distance into index 0
        dataToSend.struc.distanceKM = dataTEMP[0];                              //Shove the distance into the struct
        pc.printf("Lightning detection, distance=%dkm\r\n", dataTEMP[0]);       //DEBUGGING: Print out the distance
        pc.printf("Energy %d\r\n", ld.getEnergy());                             //DEBUGGING: Get the energy detected
        ld.clearStats();                                                        //Clear the contents and get 
    }
    dataTEMP[1] = (char)LIGHTNINGDETECTORID;                                    //Shove the detector's ID into index 1
    dataToSend.struc.detectorID = (char)LIGHTNINGDETECTORID;                    //Shove thed etector's dsitance into the struct
    dataToSend.struc.time = clocky.read();                                      //Shove the time in milliseconds into the struct
    wifi.send((char *)dataToSend.dataString, sizeof(DATA));                     //Send the data
    clocky.start();                                                             //Start the clock again
    
    //TODO: Send the struct
    
}

/**************************
SETUP - ESP8266 WIFI MODULE
***************************/
//Summary: This function will set up the transmitting ESP8266 module so that
// it will send data to the corresponding ESP8266 server with the interrupt
// data
void SetupTransmitter()
{
    wifi.init();                                                                //Initialize the ESP8266 module (using resets contained in the class)
    wifi.connect(ssid, pwd);                                                    //Connect using the SSID and Password
    //Keep re-trying to gain a connection to an access point
    while(!(wifi.is_connected()))
    {
        wait(2);
        pc.printf("Connecting to AP...\r\n");
    }
    //Keep re-trying to connect to the main device
    while(!(wifi.open(true, serverIP, 80, -1)))
    {
        wait(2);
        pc.printf("Connecting to server...\r\n");
    }
}

/**************************
SETUP - AS3935 LIGHTNING DETECTOR
***************************/
//Summary: This function sets up the lightning detector so that it may handle
// lightning strikes
void SetupLightningDetector()
{
    ld.setTuneCap(1);                                                           //500kHz
    ld.setOutdoors();                                                           //Scale it for indoors experiments
    ld.setMinimumLightnings(1);                                                 //Set it so it only needs one lightning strike to trigger
    ld.setNoiseFloor(2);                                                        
    ld.disableDisturbers();                                                     //Stop making it whine about noise
    ld.setWatchdogThreshold(2);                                                 //2 Second watchdog
    as3935INT.rise(&LightningDetected);                                         //Set it so that the interrupt function goes high when lightning is detected
}

/**************************
DEBUGGING - Receiving ESP8266 Data
***************************/
//Summary: This function will emplace any characters received by the
// ESP8266 module to the PC terminal
void dev_recv()
{
    led1 = !led1;
    wait(0.5);
    while(wifi.readable()) 
    {
        pc.putc(wifi.getc());                                                   //Emplace characters from the ESP8266 to the PC
    }
}

/**************************
DEBUGGING - Placing ESP8266 Data
***************************/
//Summary: This function will emplace any characters sent from the PC
// terminal into the ESP8266 module
void pc_recv()
{
    led4 = !led4;
    while(pc.readable()) 
    {
        wifi.putc(pc.getc());                                                   //Emplace characters from the PC to the ESP8266
    }
}