

// - - - - -
// DmxSerial - A hardware supported interface to DMX.
// DmxSerialRecv.ino: Sample DMX application for retrieving 3 DMX values:
// address 1 (red) -> PWM Port 9
// address 2 (green) -> PWM Port 6
// address 3 (blue) -> PWM Port 5
// 
// Copyright (c) 2011-2015 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// 
// Documentation and samples are available at http://www.mathertel.de/Arduino
// 25.07.2011 creation of the DmxSerial library.
// 10.09.2011 fully control the serial hardware register
//            without using the Arduino Serial (HardwareSerial) class to avoid ISR implementation conflicts.
// 01.12.2011 include file and extension changed to work with the Arduino 1.0 environment
// 28.12.2011 changed to channels 1..3 (RGB) for compatibility with the DmxSerialSend sample.
// 10.05.2012 added some lines to loop to show how to fall back to a default color when no data was received since some time.
// - - - - -

#include <DMXSerial.h>
#include <Arduino.h>
// Constants for demo program

#define LED_1 3
#define LED_2 5
#define LED_3 6
#define LED_4 9
#define LED_5 10
#define LED_6 11

void setup () 
{
  DMXSerial.init(DMXReceiver);
  
  // enable pwm outputs
  pinMode(LED_1,  OUTPUT); // sets the digital pin as output
  pinMode(LED_2,  OUTPUT);
  pinMode(LED_3,  OUTPUT);
  pinMode(LED_4,  OUTPUT);
  pinMode(LED_5,  OUTPUT);
  pinMode(LED_6,  OUTPUT);
}


void loop() 
{
    // Calculate how long no data backet was received
    unsigned long lastPacket = DMXSerial.noDataSince();
    if (lastPacket > 5000)
    {
      analogWrite(LED_1, 255);
      analogWrite(LED_2, 255);
      analogWrite(LED_3, 255);
      analogWrite(LED_4, 255);
      analogWrite(LED_5, 255);
      analogWrite(LED_6, 255);
    }
    else
    {
    
      // read recent DMX values and set pwm levels 
      analogWrite(LED_1,  DMXSerial.read(1));
      analogWrite(LED_2,  DMXSerial.read(2));
      analogWrite(LED_3,  DMXSerial.read(3));
      analogWrite(LED_4,  DMXSerial.read(4));
      analogWrite(LED_5,  DMXSerial.read(5));
      analogWrite(LED_6,  DMXSerial.read(6));
    }
 
}
