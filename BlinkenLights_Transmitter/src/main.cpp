#include <Arduino.h>
#include "DMXSerial.h" // uses the Serial port it can receive or send dmx http://www.mathertel.de/Arduino/DMXSerial.aspx or https://github.com/mathertel/DmxSerial
// #include "DMXSIMPLE.h"  // this library uses pin 3(default) or any pin capable of ditital output and can only  send dmx. Library uses timer2 https://github.com/PaulStoffregen/DmxSimple
// #include "Wire.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#include "blinkenlights.h"


#define DMX_RECEIVER false
#define DMX_CONTROLLER true
#define CHANNELS 24

#define ADC_LEVEL A0
#define ADC_DENSITY A1
#define ADC_SPEED A2
#define ADC_SPREAD A3
#define ADC_FLASH_TIME A6 // ADC_Spare_2

#define DIP_1 12
#define DIP_2 11
#define DIP_3 10
#define DIP_4 9
#define DIP_5 8
#define DIP_6 7
#define DIP_7 6
#define DIP_8 5
#define DIP_9 4

#define FLASHMODE_LED 13

/*  TINKER HERE */
#define DEBUG false
#define FLSHTM_MULTILIPLIER 500
#define FLASHTIME_CHANNEL_SIZE 6
#define FLASH_DURATION_BASE 50 // flash duration in ms
#define FLASH_DURATION_RANDOM_ADDED_MAX 100
#define FLASH_PAUSE  16 // pause between two flashes * Flashpoti(1-256) ms 4 -> 1s, 8-> 2s, ...
#define FLASH_EFFORTS 25 // number of tries to pick a light to flash
#define FLASH_BRIGHTNESS 255
#define FLASHPAUSE_RANDOMNES_PERCENT 30
#define MAX_FLASH_PAUSE 20 // seconds
/*  TINKER HERE */

class Flashtime
{
  /***
   * method update gets value of Flashtime Poti
   * returns a certain channel that has to be flashed, if idle return channel 0
   * has a state of idle, pick_Flashlight, Flashing and Flashtime_over
   * when idle wait until wait_time is over,
   * then go to pick_flashlight,
   *  flahstime has an bool array of n Channels
   *  try to pick a light that hasnt been flashed, maybe take several efforts to do so, else go to idle
   *  if found a light to flash go to flashing for a flash_duration in ms
   *
   *
   *
   *
   *
   * ***/
  public:
    Flashtime (uint8_t flashpt)
    {
      last_millis = millis();
      //flashpot = flashpt;
      last_flshtm = 0;
      channel_to_flash = 0;
      for (int i = 0; i < FLASHTIME_CHANNEL_SIZE; i++)
      {
        lights_to_flash [i] = true;
      }
      last_flashtime = millis();
      flashPause = setFlashPause(flashpt);
    }
    int getState ()
    {
      return fl_state;
    }
    bool isflashing ()
    {
      if (fl_state == flashing)
      {
        return true;
      }
      else
      {
        {
          return false;
        }
      }
    }
    int getFlashLevel ()
    {
      return FLASH_BRIGHTNESS;
    }
    int update (uint8_t flashpt)
    {
      flashpot = flashpt ;
      if (flashpot > 230)
      {
        digitalWrite(FLASHMODE_LED, LOW);
      }
      else
      {
        digitalWrite(FLASHMODE_LED, HIGH);
      }

        switch (fl_state)
        {
          case idle:
          {
            /**
             * wait for FLASH_PAUSE to be over, by comparing to u_long last_flashtime
             * then go to state pick_flashlight
             **/
            //mySerial.print ("millis - last_Flashtiome ");
            //mySerial.println(((millis()-last_flashtime)));
            //mySerial.print("Flashpause * 1024 : ");
            //mySerial.println(flashPause * 1024);
            if (((millis()-last_flashtime)) > flashPause * 1024) // flashpt < 230 -> Dead Zone of poti, Flashfunktion disabled
            {
              if (flashpot < 230 )
              {
                fl_state = pick_flashlight;
              }
               else
              {
                //mySerial.println ("DeadZone !");
              }
            }


            channel_to_flash = 0; //better safe then sorry
            //mySerial.println("idle");
            break;
          }
          case pick_flashlight :
          {
            /**
             * pick a random element in lights_to_flash [] and see if its true
             * try FLASH_EFFORTS times to find a light that has not beens flashed
             * if so go to state flashing ans set channel_to_flash to randomly chosen channel, set flash_start_time = millis()
             * else go to state flash over
             *
             **/
            int efforts = 0;
            while (efforts < FLASH_EFFORTS)
            {
              //mySerial.print("while...");
              int ch = random (0, FLASHTIME_CHANNEL_SIZE); // pick a random channel
              //mySerial.print ("try ch ");
              //mySerial.print (ch);
              if (lights_to_flash[ch])  // if its flashable...
              {
                lights_to_flash[ch] = false;

                channel_to_flash = ch;
                fl_state = flashing;          // set state to flashing
                flashDurationIndividual = FLASH_DURATION_BASE + random(0, FLASH_DURATION_RANDOM_ADDED_MAX);
                //mySerial.println(flashDurationIndividual);
                efforts = FLASH_EFFORTS ;
                flash_start_time = millis();

              }
              else          // try again, note how many efforts we took
              {
                efforts++;
              }

            }

            /** cant find another available channel to flash -> done **/
            if (fl_state != flashing)
            {
              fl_state = flash_over;
              //mySerial.println("goto flashover");
            }
            break;
          }

          case flashing :
          {
            //mySerial.print("flashing channel : ");
            //mySerial.println (channel_to_flash);
            /**
             * wait until FLASH_DURATION is over, then go back to state pick_flaslight
             *
             **/
            if ((millis() - flash_start_time) > flashDurationIndividual)
            {
              fl_state = pick_flashlight;
              //mySerial.println ("done Flashing");
            }
            break;
          }
          case flash_over :
          {
            //mySerial.print("entering flashover...  ");
            /**
             * reset lights_to_flash[] to be all true, set state to idle
             * set last_flashtime = millis()
             **/
            for (int i = 0; i < FLASHTIME_CHANNEL_SIZE; i++)
            {
              lights_to_flash [i] = true;
            }
            channel_to_flash = 0;
            last_flashtime = millis();
            flashPause = setFlashPause (flashpt);
            //mySerial.print ("Flashpause : ");
            //mySerial.println (flashPause);
            fl_state = idle;
            //mySerial.println("flashover");
            break;
          }
          default:
          {
            //mySerial.println("default");
            break;
          }
        }
      return channel_to_flash;
    }

          case flashing :
          {
            //mySerial.print("flashing channel : ");
            //mySerial.println (channel_to_flash);
            /**
             * wait until FLASH_DURATION is over, then go back to state pick_flaslight
             *
             **/
            if ((millis() - flash_start_time) > flashDurationIndividual)
            {
              fl_state = pick_flashlight;
              //mySerial.println ("done Flashing");
            }
            break;
          }
          case flash_over :
          {
            //mySerial.print("entering flashover...  ");
            /**
             * reset lights_to_flash[] to be all true, set state to idle
             * set last_flashtime = millis()
             **/
            for (int i = 0; i < FLASHTIME_CHANNEL_SIZE; i++)
            {
              lights_to_flash [i] = true;
            }
            channel_to_flash = 0;
            last_flashtime = millis();
            flashPause = setFlashPause (flashpt);
            //mySerial.print ("Flashpause : ");
            //mySerial.println (flashPause);
            fl_state = idle;
            //mySerial.println("flashover");
            break;
          }
          default:
          {
            //mySerial.println("default");
            break;
          }
        }
      return channel_to_flash;
    }


  private:
    bool lights_to_flash [FLASHTIME_CHANNEL_SIZE] ;
    int flashDurationIndividual;
    int flashPause;
    unsigned long last_millis ;
    unsigned long last_flashtime;
    unsigned long flash_start_time;
    int last_flshtm ;
    uint8_t flashpot;
    int channel_to_flash;
    enum flashstate {idle, pick_flashlight, flashing, flash_over} fl_state;

    int setFlashPause(uint8_t flashpoti)
    {
      /** Exponential **/
      // add randomness to flashpoti, flashpause follows exponential growth 3,5^x * 0. 01 * random(1, FLASHPAUSE_RANDOMNES_PERCENT)
      //float flp = pow(flashpoti, 2) * 200 ; //* (1 + 0.01 * random (1, FLASHPAUSE_RANDOMNES_PERCENT));

      /** linear **/

      int flp = map (flashpoti, 0, 255, 0, MAX_FLASH_PAUSE );
      int flpint = flp;

      /** Logadingsbums  **/
      /**
      float flp = log (flashpoti) * 1 ;
      int flpint = (int) flp;
      mySerial.print("flp : ");
      mySerial.println(flpint);
      **/

      return flpint;

    }
};

  private:
    bool lights_to_flash [FLASHTIME_CHANNEL_SIZE] ;
    int flashDurationIndividual;
    int flashPause;
    unsigned long last_millis ;
    unsigned long last_flashtime;
    unsigned long flash_start_time;
    int last_flshtm ;
    uint8_t flashpot;
    int channel_to_flash;
    enum flashstate {idle, pick_flashlight, flashing, flash_over} fl_state;

    int setFlashPause(uint8_t flashpoti)
    {
      /** Exponential **/
      // add randomness to flashpoti, flashpause follows exponential growth 3,5^x * 0. 01 * random(1, FLASHPAUSE_RANDOMNES_PERCENT)
      //float flp = pow(flashpoti, 2) * 200 ; //* (1 + 0.01 * random (1, FLASHPAUSE_RANDOMNES_PERCENT));

      /** linear **/

      int flp = map (flashpoti, 0, 255, 0, MAX_FLASH_PAUSE );
      int flpint = flp;

      /** Logadingsbums  **/
      /**
      float flp = log (flashpoti) * 1 ;
      int flpint = (int) flp;
      mySerial.print("flp : ");
      mySerial.println(flpint);
      **/

      return flpint;

    }
};

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
  pinMode(FLASHMODE_LED, OUTPUT);   // FLASHMODE_LED
  randomSeed(analogRead(A7)); // ADC SPARE 2
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
byte flashpoti = (analogRead(ADC_FLASH_TIME) >> 2);
Flashtime flash_gordon(flashpoti);

void loop()
{
  loopTimer = millis();
  flashpoti = (analogRead(ADC_FLASH_TIME) >> 2);
  int flashChannel = flash_gordon.update(flashpoti);
  if (address) // if address not 0
  {
    if (DEBUG)
    {
      lightroom.update(128, 200, 255, 255);
    }
    else
    {
      int lewel = (analogRead(ADC_LEVEL)>>2);
      byte lvl = (byte) map (lewel, 0, 255, 40, 215);
      lightroom.update(lvl, analogRead(ADC_DENSITY)>>2, analogRead(ADC_SPEED)>>2, analogRead(ADC_SPREAD)>>2);
    }
    for (uint8_t i = 0; i < CHANNELS; i++)
    {
      if ( i == flashChannel && flash_gordon.isflashing() ) // getState is true when flashing...
      {
        DMXSerial.write(i +address, flash_gordon.getFlashLevel());
      }
      else
      {
        DMXSerial.write(i + address, lightroom.getDMX(i));
      }
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
    /*
    DMXSerial.write (1,255);
    DMXSerial.write (2,200);
    DMXSerial.write (3,150);
    DMXSerial.write (4,100);
    DMXSerial.write (5,50);
    DMXSerial.write (1,0);
    */
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
