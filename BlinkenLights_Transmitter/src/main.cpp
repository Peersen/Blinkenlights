#include <Arduino.h>
#include "DMXSerial.h" // uses the Serial port it can receive or send dmx http://www.mathertel.de/Arduino/DMXSerial.aspx or https://github.com/mathertel/DmxSerial
// #include "DMXSIMPLE.h"  // this library uses pin 3(default) or any pin capable of ditital output and can only  send dmx. Library uses timer2 https://github.com/PaulStoffregen/DmxSimple
// #include "Wire.h"
#include "blinkenlights.h"

#define DMX_RECEIVER false
#define DMX_CONTROLLER true
#define CHANNELS 24

#define ADC_LEVEL A0
#define ADC_DENSITY A1
#define ADC_SPEED A2
#define ADC_SPREAD A3

#define DIP_1 12
#define DIP_2 11
#define DIP_3 10
#define DIP_4 9
#define DIP_5 8
#define DIP_6 7
#define DIP_7 6
#define DIP_8 5
#define DIP_9 4

/*  TINKER HERE */
#define DEBUG false

/*  TINKER HERE */



struct potis
{
  uint8_t level = 0 ;
  uint8_t density = 0;
  uint8_t speed = 0;
  uint8_t spread = 0;
}pots ;

void setupDIP();
int getDMXaddress();
int address ;

LightRoom lightroom(CHANNELS);
potis getPotis ();

void setup() 
{
  setupDIP();
  address = getDMXaddress(); 
  if (DMX_RECEIVER)
  {
     DMXSerial.init(DMXReceiver);
  }
  if (DMX_CONTROLLER)
  {
    DMXSerial.init(DMXController);
  }
}

uint8_t testChannels [CHANNELS];

unsigned long loopTimer;

void loop() 
{
  loopTimer = millis();

  if (address) // if address not 0
  {
    if (DEBUG)
    {
      lightroom.update(128, 200, 255, 255);
    }
    else
    {    
      lightroom.update(analogRead(ADC_LEVEL)>>2, analogRead(ADC_DENSITY)>>2, analogRead(ADC_SPEED)>>2, analogRead(ADC_SPREAD)>>2);
    }
    for (uint8_t i = 0; i < CHANNELS; i++) 
    {
      DMXSerial.write(i + address, lightroom.getDMX(i));
      //mySerial.print("DMX Adress : ");
      //mySerial.println(address);

      //mySerial.print("Dmx Channel : ");
      //mySerial.print(i+address);
      //mySerial.print(" Value : ");
      //mySerial.println(lightroom.getDMX(i));
    }
  }
  if (!(address)) // testMode at adress 0
  {
    //mySerial.print("DMX Adress : ");
    //mySerial.println(address);

    for (uint8_t i = 0; i < CHANNELS; i++)
    {
      DMXSerial.write(i+1, 255/i);        //dmx Channels start with nr 1 not 0
      testChannels[i]++;
    }
    DMXSerial.write (1,255);
    DMXSerial.write (2,200);
    DMXSerial.write (3,150);
    DMXSerial.write (4,100);
    DMXSerial.write (5,50);
    DMXSerial.write (1,0);
    delay(2); // slower fades
  }

  //mySerial.print (" ---------->>  loop took : ");
  //mySerial.println (millis() - loopTimer);
}


void setupDIP ()
{
  pinMode(DIP_1, INPUT_PULLUP);
  pinMode(DIP_2, INPUT_PULLUP);
  pinMode(DIP_3, INPUT_PULLUP);
  pinMode(DIP_4, INPUT_PULLUP);
  pinMode(DIP_5, INPUT_PULLUP);
  pinMode(DIP_6, INPUT_PULLUP);
  pinMode(DIP_7, INPUT_PULLUP);
  pinMode(DIP_8, INPUT_PULLUP);
  pinMode(DIP_9, INPUT_PULLUP);

}

int getDMXaddress ()
{
  uint16_t addr = 0;
  // readoutDIPswitch
  if (!(digitalRead(DIP_1)))
  {
    addr = addr | 1<<0 ;
  }
  if (!(digitalRead(DIP_2)))
  {
    addr = addr | 1<<1 ;
  }
  if (!(digitalRead(DIP_3)))
  {
    addr = addr | 1<<2 ;
  }
  if (!(digitalRead(DIP_4)))
  {
    addr = addr | 1<<3 ;
  }
  if (!(digitalRead(DIP_5)))
  {
    addr = addr | 1<<4 ;
  }
  if (!(digitalRead(DIP_6)))
  {
    addr = addr | 1<<5 ;
  }
  if (!(digitalRead(DIP_7)))
  {
    addr = addr | 1<<6 ;
  }
  if (!(digitalRead(DIP_8)))
  {
    addr = addr | 1<<7 ;
  }
  if (!(digitalRead(DIP_9)))
  {
    addr = addr | 1<<8 ;
  }

  return addr;
}

potis getPotis ()
{
  pots.level = analogRead(ADC_LEVEL)>>2;
  pots.speed = analogRead(ADC_SPEED)>>2;
  pots.density = analogRead(ADC_DENSITY)>>2;
  pots.spread = analogRead(ADC_SPREAD)>>2;
  if (DEBUG)
  {
    pots.level = 100;
    pots.speed = 100;
    pots.density = 100;
    pots.spread = 100;
  }

  return pots;
}