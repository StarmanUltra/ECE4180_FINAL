/*
Authors: Brian Harden + Stefan Abi-Karam + Larry Kresse
Class: ECE4180 - Fall 2020
Description: The purpose of this program is to set up a single mbed device to
 receive data (as a server) from the other 1-3 sensors. This mbed device will
 also serve to alert the user in the case of lightning.

*/

#include "mbed.h"
#include "uLCD_4DGL.h"

//DECLARATIONS: STRUCTS
// Struct to receive over TCP
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

//DECLARATIONS: GLOBAL VARIABLES
PwmOut speaker(p22);                                                            //Speaker output (needs H-Bridge for sufficient volume)
uLCD_4DGL uLCD(p13,p14,p15);                                                    //Serial tx, Serial rx, Reset pin;
DigitalOut led1(LED1);                                                          //On-board LED used for debugging purposes
DigitalOut led2(LED2);                                                          //On-board LED used for debugging purposes
DigitalOut led4(LED4);                                                          //On-board LED used for debugging purposes
Serial pc(USBTX, USBRX);                                                        //Set up the mbed USB port for debugging/monitoring
Serial wifi(p28, p27);                                                          //TX, RX of the ESP8266 WiFi module
DigitalOut wifiRST(p26);                                                        //Reset signal to the ESP8266 Device
RawSerial  bluetooth(p9,p10);                                                         //TX, RX of the bluetooth module
char ssid[32] = "GTother";                                                      //enter WiFi router ssid inside the quotes
char pwd [32] = "GeorgeP@1927";                                                 //enter WiFi router password inside the quotes
char input[4] = "0";                                                            //A char array storing data received from the lightning sensors
char buf[1024];                                                                 //A buffer for ESP8266 responses
char snd[1024];                                                                 //A buffer for sending ESP8266 commands
Timer timeoutTimer;                                                             //A timer in case of ESP8266 timeouts
unsigned int timeout;                                                           //A maximum time (in seconds) before timeout
int county;                                                                     //Count how many times XXXXXXXXXX
bool ended;                                                                     //A boolean letting us know when the ESP8266's role is terminated

//DECLARATIONS: FUNCTION PROTOTYPES
void SetupReceiver();                                                           //Sets up the receiver/server            
void SendCMD();                                                                 //Sends command to the WiFi module
void getreply();                                                                //Gathers data/replies from the WiFi module (for debugging)
void dev_recv();                                                                //Handles what happens when the module spits out data for the mbed
void pc_recv();                                                                 //DEBUGGING: Handles what happens when we type in characters from the PC

/*******************************************************************************
MAIN FUNCTION
*******************************************************************************/
int main() 
{
    wifi.baud(9600);                                                            //Set the baudrate to match the ESP8266
    wifiRST = 0;                                                                //Reset the ESP8266
    wait(0.5);                                                                  //Give it a little time to fully reset
    wifiRST = 1;                                                                //Raise the reset pin
    wait(1);                                                                    //Give it a second to re-initialize
    //Set up the receiver
    SetupReceiver();
    wifi.attach(&dev_recv, Serial::RxIrq);                                      //After setting up, attach the WiFi module to the appropriate interrupt routines
    //Main loop (don't do anything)
    while(1) 
    {
        //Do something lol
        led2 = !led2;
        wait(1);
        //sleep(1);
    }
}


/**************************
ESP8266 - SETUP
***************************/
//Summary: This function will set up the transmitting ESP8266 module so that
// it will send data to the corresponding ESP8266 server with the interrupt
// data
void SetupReceiver()
{
    strcpy(snd,".\r\n.\r\n");                                                   //Make sure that we start on a newline
    SendCMD();      //Send written command to the ESP8266
    timeout = 4;    //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    wait(0.1);      //Give it time to transmit
    
    strcpy(snd,"node.restart()\r\n");                                           //Enter the reset sequence
    SendCMD();      //Send written command to the ESP8266
    //timeout = 4;  //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    wait(0.1);      //Give it time to transmit
    
    strcpy(snd,"wifi.setmode(wifi.STATION)\r\n");                               //Set up the device as a station
    SendCMD();      //Send written command to the ESP8266
    //timeout = 4;  //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    wait(0.1);        //Give it time to transmit
    
    pc.printf("ssid = %s   pwd = %s\r\n",ssid,pwd);                             //Enter in the password to connect to AP
    strcpy(snd, "wifi.sta.config(\"");
    strcat(snd, ssid);
    strcat(snd, "\",\"");
    strcat(snd, pwd);
    strcat(snd, "\")\r\n");
    SendCMD();      //Send written command to the ESP8266
    timeout=10;     //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    wait(0.1);      //Give it a little time
    
    strcpy(snd, "print(wifi.sta.getip())\r\n");                                 //Print out the IPs to the PC
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    strcpy(snd, "srv=net.createServer(net.TCP)\r\n");                           //Create a TCP server on port 80
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    strcpy(snd, "srv:listen(80,function(conn)\r\n");                            //Make the server listen for connections
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    strcpy(snd, "conn:on(\"receive\",function(conn,payload)\r\n");              //Make the server react to receiving a packet (print the contents)
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    strcpy(snd, "print(payload)\r\n");                                          //Print just the payload of the TCP packets
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    /*
    strcpy(snd, "conn:send(\"<h1>test</h1>\")\r\n");                            //DEBUG: Send some html to test the server functionality
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    */
    
    strcpy(snd, "end)\r\n");                                                    //End function
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
    strcpy(snd, "end)\r\n");                                                    //End setup command
    SendCMD();      //Send written command to the ESP8266
    timeout=3;      //Set the timeout interval (in seconds)
    getreply();     //Get a reply
    pc.printf(buf); //Print the returned statement to the PC
    
}

/**************************
ESP8266 - SEND COMMAND
***************************/
//Summary: This function sends a LUA command to the ESP8266 so that it may
// communicate with the ESP8266 wifi server
void SendCMD()
{
    wifi.printf("%s", snd);                                                     //Send the command to the ESP8266 module
    //pc.printf(snd);                                                           //DEBUGGING: Send it to the terminal to monitor commands
}

/**************************
ESP8266 - GET REPLY
***************************/
//Summary: This function will be used to receive any replies from the 
// ESP8266 module 
void getreply()
{
    memset(buf, '\0', sizeof(buf));
    timeoutTimer.start();
    ended = 0;
    county = 0;
    while(!ended) {
        if(wifi.readable()) {
            buf[county] = wifi.getc();
            county++;
        }
        if(timeoutTimer.read() > timeout) {
            ended = 1;
            timeoutTimer.stop();
            timeoutTimer.reset();
        }
    }
}

/**************************
Receiving ESP8266 Data
***************************/
//Summary: This function will emplace any characters received by the
// ESP8266 module to the PC terminal. Essentially, this will place all
// incoming network traffic into the appropriate variables
void dev_recv()
{
    //Make the speaker make a warning sound
    speaker.period(1.0/500.0);                                                  // 500hz period
    //Make it beep 3 times
    for(int i = 0; i < 3; i++)
    {
        speaker =0.5;                                                           //50% duty cycle - max volume
        wait(0.1);
        speaker=0.0;                                                            //Turn off audio
        wait(0.1);
    }
    //Each packet should only contain 4 char's
    union rawReceivedData receivedPacket;
    for(int dataCounter = 0; dataCounter < sizeof(DATA); dataCounter++)
    {
        if(wifi.readable())
        {
            receivedPacket.dataString[dataCounter] = wifi.getc();
        }
        else
        {
            break;
        }
    }
    pc.printf("Message from #%d:\n", (int)receivedPacket.struc.detectorID);
    pc.printf("WARNING! Lightning detected at %dcm!\r\n", (int)receivedPacket.struc.distanceKM);
    pc.printf("Finished!\r\n");
    //Write out to the uLCD
    uLCD.cls();
    uLCD.printf("Message from #%d:\n", (int)receivedPacket.struc.detectorID);
    uLCD.printf("WARNING! Lightning detected at %dcm!\r\n", (int)receivedPacket.struc.distanceKM);
    //Write out to the bluetooth module        
    bluetooth.printf("Message from #%d:\n", (int)receivedPacket.struc.detectorID);
    bluetooth.printf("WARNING! Lightning detected at %dcm!\r\n", (int)receivedPacket.struc.distanceKM);
    
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
        wifi.putc(pc.getc());                                                    //Emplace characters from the PC to the ESP8266
    }
}