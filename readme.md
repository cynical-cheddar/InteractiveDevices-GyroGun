# The Gyro Gun

The Gyro Gun is an Arduino-based I/O device, quickly prototyped in a handful of afternoons for  University of Bristol
COMS30054/COMSM0058 - Interactive Devices.

It utilises an Arduino Micro, RFM69HCW Radio Breakout Board, and MPU-6050 to create an orientation-based game controller.

The Gyro Gun utilises a 9V D battery. This directly powers the haptic feedback solenoid. The voltage is stepped down through a DC-DC buck converter to 5V, powering the Arduino and relays.


![alt text](https://i.imgur.com/YfmnCgU.png)
![alt text](https://i.imgur.com/hHACRT0.png)

# ASTEROID BLASTER


![alt text](https://i.imgur.com/IOwuqYb.png)


Asteroid Blaster uses the Gyro Gun as its I/O device.
Blast as many asteroids as you can within 66 seconds!

Asteroid Blaster runs in the Unity Editor. It requires the Ardity package to be installed to manage serial communication. 




