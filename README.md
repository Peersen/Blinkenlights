# Blinkenlights

An arduino is controlled by 4 potentiometers controlling:
- overall Level
- speed of the fading lights
- range how far the lights fade out from Base LEvel
- the amount of lights doing crazy stuff

The PCB includes
- Dip switch
- spare poti connectors for later use
- I2C Pins
- reset Pins
- Screw terminals for Vin and 5v rail of the arduino


To Start
- if you use Platform IO: make sure to change " lib_extra_dirs = D:\ArduinoSketches\libraries" in PlatformIO.ini
- install DMXserial Librarie by Matthias Hertel
