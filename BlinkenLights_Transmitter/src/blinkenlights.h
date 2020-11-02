#ifndef BLINKENLIGHTS_H
#define BLINKENLIGHTS_H

/* TINKER HERE  */ 
#define ARRAY_SIZE 24 // max 300 Objects on an Arduino Nano
#define DEBUG true    // Debug and status information via Software serial
#define BAUD 57600   // baud of Software Serial

#define RANGE_DIVIDER 2 // 1 -> pots.range / 1  2-> pots.range / 2
#define SPEED_MULTIPLIER 5 // 
#define SPEED_DIVIDER 20 // Higher = faster
#define SPEED_STEPTIME_RANDOM_ADDED_MS 4
/* TINKER HERE  */


#include "Arduino.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

// Struct that is handed over between Lights and Lightroom Objects
  struct settings
  {
    uint8_t level;
    uint8_t density;
    uint8_t speed;
    uint8_t range;
  };

enum lightState
  {
    idle, pickParams, fadeToTarget, fadeBackFromTarget // 0, 1, 2, 3 
  };

/*
a light object gives back its dmx value or state (idle or in fading or picking parameters) to the lightroom object
it accepts the 4 inputs from the UI: level, density, spread(of level), speed
if it decides based on the density parameter to start a fade it does that and after that listens again to incomming parameters from the lightroom object
it then computes its dmx value which can be called by lightroom object
*/
class Light
{
public:
  Light()
  {
    state = idle ;
    randomSeed(analogRead(A0));
    // maybe set a standard deviation from base level here to have an individual level for this light
  }
  void tick(settings pots)
  {
    //pots = pots;
      if (state)  // not idle
      { //increment state machine
        switch (state)
            {
            case pickParams:
                // pick paremters
                setFade(pots);
                state = fadeToTarget;
                params.lastMillis = millis();
                break;
            case fadeToTarget:
                // in- decrement level based on params 
                if (fadeOut())
                {
                  return;
                }
                else
                {
                  state = fadeBackFromTarget;
                  params.lastMillis = millis();
                }                
                break;
            case fadeBackFromTarget :
                // in- decrement level based on params
                if (fadeBack())
                {
                  return;
                }
                else
                {
                  state = idle;
                }
                break;
            default:
                break;
            }
      }
      else // if idle
      {
          params.level = pots.level; 
          // 
      } 
  }
  uint8_t getLevel ()
  {
      return params.level;
  }
  uint8_t getState ()
  {
    return state;
  }
  uint8_t getBaseLevel ()
  {
    return params.baseLevel ;
  }
  bool getfadeDirection()
  {
    return params.fadeDirection;
  }
  uint16_t getFadeOutSpeed ()
  {
    return params.fadeOutSpeed;
  }
  uint16_t getFadeBackSpeed ()
  {
    return params.fadeBackSpeed;
  }
  uint8_t getfadeTarget ()
  {
    return params.fadeTarget;
  }

  void setState(lightState statetoset)
  {
      state = statetoset;
  }

  private:
  void setFade (settings pots) 
  {
    // set Base Level
    params.baseLevel = pots.level;
    params.level = pots.level;
    // Pick a fade direction 
    if (random(0,2)) // random()`s upper limit is exclusive, random (0,2) gives 0 and 1
    {
      params.fadeDirection = true;
    }
    else
    {
      params.fadeDirection = false;
    }
        
    // pick a random range to fade, reduce the fade Size by RANGE_DIVIDER overall, then add some randomnes to the resulting value
    int range = int ((pots.range / RANGE_DIVIDER) * (0.01 * random (10, 101)));
    
    if (!params.fadeDirection) // if negative
    {
      range = range * -1 ;
    }
    // range is now a value from +-0 to +-255 -> add it to baselevel
    //mySerial.print ("Range : ");
    //mySerial.println (range);
    
    int ft = params.baseLevel + range ;
    if (ft <0)
    {
      ft = 0;
    }
    if (ft > 255)
    {
      ft = 255;
    }
    //mySerial.print ("ft : ");
    //mySerial.println (ft);
    
    params.fadeTarget = byte(ft);
    
    
    /******FADE SPEED*******/

    params.fadeOutSpeed = (255 - pots.speed) /SPEED_DIVIDER  + random(0, SPEED_STEPTIME_RANDOM_ADDED_MS) ;
    params.fadeBackSpeed = (255 - pots.speed) /SPEED_DIVIDER + random(0, SPEED_STEPTIME_RANDOM_ADDED_MS);


   
    params.lastMillis = millis();
  }
  bool fadeOut()
  {
  // Returns true if it hasnt reached params.fadeTarget
  // False if target reached, false tells state machine to switch to state:fadeBackFromTarget
    if(!(params.level == params.fadeTarget))
    {    
      if ((millis() - params.lastMillis) > params.fadeOutSpeed ) // if its time to makle a step
      {
        if (params.fadeDirection)
        {
          params.level++ ;
        }
        else
        {
          params.level--;
        }        
        params.lastMillis = millis();
        return true;
      }    
      else
      {
        return true; // wait until next time
      }
    }
    else
    {
      return false;
    }
  }
  bool fadeBack ()
  {
  // Return true if its still fading back to params.baseLevel
  // false if back to BaseLevel, tells statemachine to switch to idle
    if (!(params.level == params.baseLevel)) // if were not back yet
    {
      if ((millis()-params.lastMillis) > params.fadeBackSpeed) // if its time to make a step
      {
        if (params.fadeDirection)
        {
          params.level--;
        }
        else
        {
          params.level++;
        }
        params.lastMillis = millis();
        return true;
      }
      else
      {
        return true; // wait until next time to make a step
      }
    }
    else
    {
      return false;
    }
    
  }

  lightState state;

  struct parameter 
  {
    uint8_t baseLevel;      // base value from Pots level to fade from and back
    uint8_t level;          // actual level of the light
    bool fadeDirection;     // 0 or 1 (fade down or up)
    uint16_t fadeOutSpeed;   // stepspeed of fade out in milliseconds, randomized by SPEED_MULTIPLIER and internal int speedupfacor
    uint16_t fadeBackSpeed;  // same here...
    uint8_t fadeTarget;     // target value to fade to
    unsigned long lastMillis;
  } params ;
};



class LightRoom
{
  public:
    //constructor
    LightRoom(uint8_t lightsCount) // init with CHANNELS 1-ARRAY_SIZE
    {
      randomSeed(analogRead(0));
      pots.level = 0;
      pots.density = 0;
      pots.speed = 0;
      pots.range = 0;
      lightsCounter = lightsCount -1 ; // lights counter goes from 0 to CHANNELS -1
      if (DEBUG)
      {
        setupSS();
      }  
    }
    void update (uint8_t level, uint8_t density, uint8_t speed, uint8_t range)
    {
      pots.level = level;       // base level for all lights
      pots.density = density;   // number of lights doing something
      pots.range = range;       // how far a fade moves from base level in average
      pots.speed = speed;       // how quick a light moves from and to base level in average

      // get state of all lights and see if we need to fire up more lightruns based on density pot
      uint8_t lightsActive = 0;
      for (uint8_t i = 0; i < lightsCounter; i++ )
      {
        if (lightArr[i].getState()) // if its alredy active
        {
          lightsActive++;
        }
      }

      // fireUp a random light that isnt already running, if necessary. 
      // tell it the current pot values besides density by tick() method
      uint8_t lightsToBeActive = map (pots.density, 0, 255, 0, lightsCounter+1 ); // map(value, fromLow, fromHigh, toLow, toHigh)
      mySerial.print ("LightsTobE active : ");
      mySerial.println (lightsToBeActive);
      mySerial.print("lightsCoutner : ");
      mySerial.println (lightsCounter);
// --->>> Loop of Death if density = 255 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      while (lightsToBeActive > lightsActive)
      {
        // pick a random light
        uint8_t randomLight = random (0, lightsCounter +1); //random(min, max)
        //mySerial.print ("Random Light : ");
        //mySerial.println (randomLight);
        
        if (!(lightArr[randomLight].getState())) // if its idle
        { 
          //mySerial.println ("... is set Active ");
          lightArr[randomLight].setState(pickParams); // 1 means state=pickParams
          lightsActive++;
        }
        //mySerial.print ("Lights Active : ");
        //mySerial.println (lightsActive);
        //mySerial.print ("Lights to be Active : ");
        //mySerial.println (lightsToBeActive);
      }
      // update all lights in lightArr by tick();
      for (uint8_t i = 0; i <= lightsCounter; i++ )
      {
        lightArr[i].tick(pots);
      }
      //debug ();

      //mySerial.print ("Lights to be active : ");
      //mySerial.println (lightsToBeActive);

      //mySerial.print ("LightsCounter : ");
      //mySerial.println (lightsCounter);
    }
    uint8_t getDMX(uint16_t lightNr) // 0 to CHANNELS-1
    {
        return lightArr[lightNr].getLevel();
    }
    
  private:
    uint8_t lightsActive = 0;
    settings pots ;
    uint8_t lightsCounter;
    // Array of Light Object size of the Array makes a HUGHE difference in Ram usage... 
    Light lightArr [ARRAY_SIZE]; // max 300 Objects on an Arduino Nano
    void setupSS ()
    {
    
    /*
      Software serial multple serial test

    Receives from the hardware serial, sends to software serial.
    Receives from software serial, sends to hardware serial.

    The circuit:
    * RX is digital pin 2 (connect to TX of other device)
    * TX is digital pin 3 (connect to RX of other device)

    Note:
    Not all pins on the Mega and Mega 2560 support change interrupts,
    so only the following can be used for RX:
    10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

    Not all pins on the Leonardo support change interrupts,
    so only the following can be used for RX:
    8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

    created back in the mists of time
    modified 25 May 2012
    by Tom Igoe
    based on Mikal Hart's example

    This example code is in the public domain.

    */
      // SoftwareSerial mySerial(2, 3); // RX, TX
      mySerial.begin(BAUD);
      mySerial.println("debugging Startet");
    }

    void debug()
    {
      if (DEBUG)
      {/*
        mySerial.println ("debuginfo Lightroom");
        mySerial.print ("Nr of Lights Active :  ");
        mySerial.println (lightsActive);
        mySerial.print ("pots Level :  ");
        mySerial.println (pots.level);
        mySerial.print ("pots density :  ");
        mySerial.println (pots.density);
        mySerial.print ("pots Range :  ");
        mySerial.println (pots.range);
        mySerial.print ("pots Speed :  ");
        mySerial.println (pots.speed);
        */

        for (int i = 0; i < ARRAY_SIZE; i++)
        {
          //mySerial.println ("#######");
          mySerial.print ("Light Nr : ");
          mySerial.println (i);
          //mySerial.print ("state : ");
          //mySerial.println (lightArr[i].getState());
          //mySerial.print ("level : ");
          //mySerial.println (lightArr[i].getLevel());
          //mySerial.print ("fadeDirection up ? : ");
          //mySerial.println (lightArr[i].getfadeDirection());
          mySerial.print ("Fade Out Speed  : ");
          mySerial.println (lightArr[i].getFadeOutSpeed());
          mySerial.print ("Fade Back Speed  : ");
          mySerial.println (lightArr[i].getFadeBackSpeed());
          //mySerial.print ("Fade Target  : ");
          //mySerial.println (lightArr[i].getfadeTarget());
          //mySerial.println ("##############################################################");

        }
      }
    }
};
#endif
/*
{
    uint8_t baseLevel;      // base value from Pots level to fade from and back
    uint8_t level;          // actual level of the light
    bool fadeDirection;     // 0 or 1 (fade down or up)
    uint16_t fadeOutSpeed;   // stepspeed of fade out in milliseconds, randomized by SPEED_MULTIPLIER and internal int speedupfacor
    uint16_t fadeBackSpeed;  // same here...
    uint8_t fadeTarget;     // target value to fade to
    unsigned long lastMillis;
  } params ;
*/