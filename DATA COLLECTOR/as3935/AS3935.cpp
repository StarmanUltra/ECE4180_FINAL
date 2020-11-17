/*
AS3935.cpp - AS3935 Franklin Lightning Sensor™ IC by AMS library
Copyright (c) 2012 Raivis Rengelis (raivis [at] rrkb.lv). All rights reserved.
Porté sur MBED par Valentin, version I2C

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

#include "AS3935.h"
#include "pinmap.h"

unsigned long sgIntrPulseCount = 0; 


AS3935::AS3935(PinName mosi, PinName miso, PinName sclk, PinName cs, const char* name, int hz) :  m_Spi(mosi, miso, sclk), m_Cs(cs, 1),  m_FREQ(hz)
{
 
    //Enable the internal pull-up resistor on MISO
    pin_mode(miso, PullUp);

    //Configure the SPI bus
    m_Spi.format(8, 1);
    printf("spi bus frequency set to %d hz\r\n",hz);
    m_Spi.frequency(hz);
  
}
 
char AS3935::_SPITransfer2(char high, char low)
{
    m_Cs = 0;
    m_Spi.write(high);
    char regval = m_Spi.write(low);
    m_Cs = 1;
    return regval;  
}

char AS3935::_rawRegisterRead(char reg)
{
    return _SPITransfer2((reg & 0x3F) | 0x40, 0);
}


char AS3935::_ffsz(char mask)
{
    char i = 0;
       
    while(!(mask & 1)) {
           mask >>= 1;
          i++;
    }
 
    return i;
}

void AS3935::registerWrite(char reg, char mask, char data)
{
   
    char regval;
    regval = _rawRegisterRead(reg);
    regval &= ~(mask);
    regval |= (data << (_ffsz(mask)));
    m_Cs = 0;
    m_Spi.write(reg);
    m_Spi.write(regval);
    //printf("raw transfer for reg %x is 0x%02x\r\n", reg, regval);
    m_Cs = 1;
    wait_ms(2);
}

char AS3935::registerRead(char reg, char mask)
{
    char regval;
    regval = _rawRegisterRead(reg);
    // printf("raw regval is 0x%02x\r\n", regval);
    regval = regval & mask;
    regval >>= (_ffsz(mask));
    return regval;
}

void AS3935::presetDefault()
 {
    m_Cs = 0;
    m_Spi.write(0x3C);
    m_Spi.write(0x96);
    m_Cs = 1;
    wait_ms(2);
}

void AS3935::init()
 {
    presetDefault();
    registerWrite(AS3935_WDTH, 0x04);  // set WDTH level to 4 
}
 
void AS3935::powerDown()
{
    registerWrite(AS3935_PWD,1);
}



int AS3935::interruptSource()
{
    return registerRead(AS3935_INT);
}

void AS3935::disableDisturbers()
{
    registerWrite(AS3935_MASK_DIST,1);
}

void AS3935::enableDisturbers()
{
    registerWrite(AS3935_MASK_DIST,0);
} 

int AS3935::getMinimumLightnings()
{
    return registerRead(AS3935_MIN_NUM_LIGH);
}

int AS3935::setMinimumLightnings(int minlightning)
{
    registerWrite(AS3935_MIN_NUM_LIGH,minlightning);
    return getMinimumLightnings();
}

int AS3935::lightningDistanceKm()
{
    return registerRead(AS3935_DISTANCE);
}

void AS3935::setIndoors()
{
    registerWrite(AS3935_AFE_GB,AS3935_AFE_INDOOR);
}

int AS3935::getGain()
{
    return registerRead(AS3935_AFE_GB);
}

void AS3935::setOutdoors()
{
    registerWrite(AS3935_AFE_GB,AS3935_AFE_OUTDOOR);
}

int AS3935::getNoiseFloor()
{
    return registerRead(AS3935_NF_LEV);
}

int AS3935::setNoiseFloor(int noisefloor)
{
    registerWrite(AS3935_NF_LEV,noisefloor);
    return getNoiseFloor();
}

int AS3935::getSpikeRejection()
{
    return registerRead(AS3935_SREJ);
}

int AS3935::setSpikeRejection(int srej)
{
    registerWrite(AS3935_SREJ, srej);
    return getSpikeRejection();
}

int AS3935::getWatchdogThreshold()
{
    return registerRead(AS3935_WDTH);
}

int AS3935::setWatchdogThreshold(int wdth)
{
    registerWrite(AS3935_WDTH,wdth);
    return getWatchdogThreshold();
}

int AS3935::getTuneCap()
{
    return registerRead(AS3935_TUN_CAP);    
}
        
int AS3935::setTuneCap(int cap)
{
    registerWrite(AS3935_TUN_CAP,cap);
    return getTuneCap();   
}

void AS3935::clearStats()
{
    registerWrite(AS3935_CL_STAT,1);
    registerWrite(AS3935_CL_STAT,0);
    registerWrite(AS3935_CL_STAT,1);
}
    
static void intrPulseCntr(void)    {sgIntrPulseCount++;} 

int AS3935::calibrateRCOs (InterruptIn &intrIn)
{
    int rc;
    uint8_t trco;
    uint8_t srco;
    Timer pulseTimer;
    int timeNow; 
    unsigned long measFreq; 
    
    intrIn.rise(intrPulseCntr); 
    
    _SPITransfer2(0x3D, 0x96);                        // send command to calibrate the internal RC oscillators           
    registerWrite(AS3935_DISP_TRCO, 1);               // put TRCO on the IRQ line for measurement   
    wait_ms(20);                                      // wait for the chip to output the frequency, ususally ~2 ms 
    
    pulseTimer.reset(); 
    pulseTimer.start(); 
    sgIntrPulseCount = 0;                             // reset the interrupt counter which serves as our frequency\pulse counter     
    
    timeNow = 0; 
    while (timeNow < 500)                             // wait for 0.5 seconds
    {
      timeNow = pulseTimer.read_ms(); 
    }
    
    registerWrite(AS3935_DISP_TRCO, 0);               // stop the output of the frequncy on IRQ line       
    measFreq = sgIntrPulseCount << 1;                 // calculate the measure frequency based upon period of capture and freq scaler

    printf("timer RCO: %ld Hz\n\r", measFreq);
    
    trco=registerRead(0x3A, 0x80);                    // Read out Calibration of TRCO done
    srco=registerRead(0x3B, 0x80);                    // Readout Calibration of SRCO done
    if(trco != 0x00 && srco != 0x00)
    {
        rc = 1;
        printf("cal is done\r\n");
    }
    else
    {
        printf("cal is not done\r\n");
        rc = 0;
    }

    return rc;
}


unsigned long AS3935::tuneAntenna(InterruptIn &intrIn)
{
#define ANTENA_RES_FREQ     (unsigned long)500000
Timer pulseTimer;
int timeNow; 
unsigned long measFreq; 
unsigned long measFreqBest = 0; 
unsigned char tunCapCnt    = 0; 
unsigned char tunCapBest   = 0;     
unsigned long minError     = ANTENA_RES_FREQ; 
unsigned long error; 
    
    intrIn.rise(intrPulseCntr); 
    _SPITransfer2(3, 0x80);                             // set frequency division to 64  
    
    for (tunCapCnt = 0; tunCapCnt < 16; ++tunCapCnt)    // loop for all possible values of the tuning capacitor
    {
      _SPITransfer2(8, 0x80+tunCapCnt);                 // set the tuning cap and have the frequency output to the IRQ line 
      wait_ms(20);                                      // wait for the chip to output the frequency, ususally ~2 ms 

      pulseTimer.reset(); 
      pulseTimer.start(); 
      sgIntrPulseCount = 0;                             // reset the interrupt counter which serves as our frequency\pulse counter     
    
      timeNow = 0; 
      while (timeNow < 500)                             // wait for 0.5 seconds
      {
        timeNow = pulseTimer.read_ms(); 
      }
      
      _SPITransfer2(8, 0x00);                           // stop the output of the frequncy on IRQ line 
      
      measFreq = sgIntrPulseCount << 7;                 // calulate the measure frequency based upon period of capture and freq scaler
      
      if (measFreq < ANTENA_RES_FREQ)                   // calculate error between actual and desired frequency 
        error = ANTENA_RES_FREQ - measFreq; 
      else 
        error = measFreq - ANTENA_RES_FREQ; 
        
      if (error < minError)                             // update the best capacitor tuning so far 
      {
        tunCapBest = tunCapCnt; 
        minError = error; 
        measFreqBest = measFreq; 
      }
      
      printf("sgIntrCount[%ld] measFreq[%ld] timeNow[%ld] tunCapBest[%d]\n\r", sgIntrPulseCount, measFreq, timeNow, tunCapBest);
    }
    setTuneCap(tunCapBest); //  500kHz);                // set the best capacitor tuning that was found 
    return measFreqBest; 
}


void AS3935::_rawRegisterRead(unsigned char reg, unsigned char mask, unsigned char *rxBuff, unsigned char numBytes)
{
    mask = mask; // unused 
    
    m_Cs = 0;
    m_Spi.write((reg & 0x3F) | 0x40);
    
    for (unsigned char idx = 0; idx < numBytes; ++idx) 
    {
        rxBuff[idx] = m_Spi.write(0);
    }
    m_Cs = 1;
}

unsigned long AS3935::getEnergy(void)
{
    unsigned long retVal; 
    unsigned char rxBuff[3];
    
    #if 0 
    rxBuff[0] = registerRead(AS3935_S_LIG_L); 
    rxBuff[1] = registerRead(AS3935_S_LIG_M); 
    rxBuff[2] = registerRead(AS3935_S_LIG_MM); 
    #else 
    _rawRegisterRead(AS3935_S_LIG_L, rxBuff, 3); 
    #endif 
    
    retVal = ((unsigned long)rxBuff[2] << 16) | ((unsigned long)rxBuff[1] << 8) | (unsigned long)rxBuff[0]; 
    retVal &= 0x001fffff; 
    return retVal; 
}

bool AS3935::getConfigRegisters(unsigned char *pBuff, unsigned char buffLen)
{
    unsigned char cnt = 0; 
    
    if (NULL == pBuff)    
        return false; 
    
    for (cnt = 0; cnt < buffLen && cnt < MAX_CONFIG_REGS; ++cnt) 
    {
        pBuff[cnt] = _rawRegisterRead(cnt); 
    }
    return true; 
}    



