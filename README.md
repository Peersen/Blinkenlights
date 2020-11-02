# Blinkenlights

An Arduino Nano sending DMX to a Driver Chip making several Dimmerchannels blink in a random but controlled manner.
It uses four potentiometers controlling:
- overall Level
- speed of the fading lights
- range how far the lights fade out from Base LEvel
- the amount of lights doing crazy stuff

PCB
- one sided for home-etching
- Dip switch for dmx address
- selector jumper to switch between sending and receiving dmx
- spare poti connectors for later use
- I2C Pins
- reset Pins
- Screw terminals for Vin and 5v rail of the arduino and DMX signal from the SN75176 tranceiverchip

Other projects in this repository help to troubleshoot the main project
- a dmx receiver making six Leds blink
- a softwareSerial receiving arduino that sends received strings to the arduinos USB COM port

To Start
- if you use Platform IO: make sure to change " lib_extra_dirs = D:\ArduinoSketches\libraries" in PlatformIO.ini
- install DMXserial Librarie by Matthias Hertel:
  http://www.mathertel.de/Arduino/DMXSerial.aspx or https://github.com/mathertel/DmxSerial
