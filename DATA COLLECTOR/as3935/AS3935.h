/*
AS3935.h - AS3935 Franklin Lightning Sensor™ IC by AMS library
Copyright (c) 2012 Raivis Rengelis (raivis [at] rrkb.lv). All rights reserved.
Portée sur MBED par Valentin

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef _AS3935_H
#define _AS3935_H

#include "mbed.h"
#include <stdint.h>

//#include "i2c.hpp"

// register access macros - register address, bitmask
#define AS3935_AFE_GB 0x00, 0x3E
#define AS3935_PWD 0x00, 0x01
#define AS3935_NF_LEV 0x01, 0x70
#define AS3935_WDTH 0x01, 0x0F
#define AS3935_CL_STAT 0x02, 0x40
#define AS3935_MIN_NUM_LIGH 0x02, 0x30
#define AS3935_SREJ 0x02, 0x0F
#define AS3935_LCO_FDIV 0x03, 0xC0
#define AS3935_MASK_DIST 0x03, 0x20
#define AS3935_INT 0x03, 0x0F
#define AS3935_S_LIG_L  0x04, 0xff
#define AS3935_S_LIG_M  0x05, 0xff
#define AS3935_S_LIG_MM 0x06, 0x1f
#define AS3935_DISTANCE 0x07, 0x3F
#define AS3935_DISP_LCO 0x08, 0x80
#define AS3935_DISP_SRCO 0x08, 0x40
#define AS3935_DISP_TRCO 0x08, 0x20
#define AS3935_TUN_CAP 0x08, 0x0F
#define MAX_CONFIG_REGS     9 

// other constants
#define AS3935_AFE_INDOOR 0x12 
#define AS3935_AFE_OUTDOOR 0x0E

#define AS3935_EVENT_NOISE     1 
#define AS3935_EVENT_DISTURBER 4 
#define AS3935_EVENT_LIGHTNING 8 




class AS3935 {
    
    public:
             
    /** Create a virtual file system for accessing SD/MMC cards via SPI
     *
     * @param mosi The SPI data out pin.
     * @param miso The SPI data in pin.
     * @param sclk The SPI clock pin.
     * @param cs The SPI chip select pin.
     * @param name The name used to access the spi bus.
     * @param hz The SPI bus frequency (defaults to 1MHz).
    */
     
    AS3935(PinName mosi, PinName miso, PinName sclk, PinName cs, const char* name, int hz = 2000000);
     
    //destruction         
    //~AS3935();   
        
    //write to specified register specified data using specified bitmask,     
    //the rest of the register remains intact
    void registerWrite(char reg, char mask, char data);
        
    //read specified register using specified bitmask and return value aligned     
    //to lsb, i.e. if value to be read is in a middle of register, function     
    //reads register and then aligns lsb of value to lsb of byte
    char registerRead(char reg, char mask);
        
    //reset all the registers on chip to default values
    void reset();
        
    //set preset defaults
    void presetDefault();
        
    //initialization
    void init();
    
    // replicate the acurite sequece
    void acurite();
      
    //put chip into power down mode
    void powerDown();
    
    //bring chip out of power down mode and perform RCO calibration
    void powerUp();
    
    //return interrupt source, bitmask, 0b0001 - noise, 0b0100 - disturber,     
    //0b1000 - lightning
    int interruptSource();
        
    //disable indication of disturbers
    void disableDisturbers();
    
    //enable indication of distrubers
    void enableDisturbers();
    
    //return number of lightnings that need to be detected in 17 minute period     
    //before interrupt is issued
    int getMinimumLightnings();
    
    //set number of lightnings that need to be detected in 17 minute period     
    //before interrupt is issued
    int setMinimumLightnings(int minlightning);
    
    //return distance to lightning in kilometers, 1 means storm is overhead,     
    //63 means it is too far to reliably calculate distance
    int lightningDistanceKm();
        
    // load gain preset to operate indoors
    void setIndoors();
        
    //load gain preset to operate outdoors
    void setOutdoors();
    
    //get gain preset 
    int getGain();
        
    //return noise floor setting - refer to datasheet for meaning and range
    int getNoiseFloor();
        
    //set noise floor setting
    int setNoiseFloor(int noisefloor);
        
    //return spike rejection value - refer to datasheet for meaning and range
    int getSpikeRejection();
        
    //set spike rejection value
    int setSpikeRejection(int srej);
        
    //return watchdog threshold value - refer to datasheet for meaning and range
    int getWatchdogThreshold();
        
    //set watchdog threshold value
    int setWatchdogThreshold(int wdth);
        
    //return tune Capacity value 
    int getTuneCap();
        
    //set tune Capacity value
    int setTuneCap(int cap);
        
    //clear internal accumulated lightning statistics
    void clearStats();

    int calibrateRCOs (InterruptIn &intrIn);

    unsigned long tuneAntenna(InterruptIn &intrIn); 
    unsigned long getEnergy(void);
    bool          getConfigRegisters(unsigned char *pBuff, unsigned char buffLen);

    
    /** Attach a function, lightning interrupt
     * @param fptr pointer to a void function, or 0 to set as none
     */
    void attach(void (*fptr)(void)) { 
//        _func.attach(fptr);
    }
    /** Attach a member function, lightning interrupt
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach(T *tptr, void (T::*mptr)(void)) { 
        _func.attach(tptr, mptr); 
    }
    
         
    private:
    //I2C i2c;   
    //DigitalOut _irq;
    SPI m_Spi;
    DigitalOut m_Cs;
    //InterruptIn m_Cd;
    
    int m_CdAssert;
    const int m_FREQ;
    int _adress;
    FunctionPointer _func;
    char _rawRegisterRead(char reg);
    void _rawRegisterRead(unsigned char reg, unsigned char mask, unsigned char *rxBuff, unsigned char numBytes);
    char _SPITransfer2(char high, char low);
    char _ffsz(char mask);
    



};

/* !_AS3935_H_ */
#endif