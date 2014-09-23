aCap
====

An eletronic cap based on the LilyPad Arduino for detecting and transmitting head movements to emulate a computer mouse (and do some other stuff ;-).

Hardware
--------

- 1 x LilyPad Arduino Main Board (http://lilypadarduino.org/?p=128);
- 1 x LilyPad RGB LED (http://lilypadarduino.org/?p=519);
- 1 x LilyPad Buzzer (http://lilypadarduino.org/?p=436);
- 1 x LilyPad Vibe Board (http://lilypadarduino.org/?p=514);
- 1 x LilyPad LiPower module (http://www.sparkfun.com/products/11260) and a Polymer Lithium Ion Battery (for example, a 1000mAh will work: http://www.sparkfun.com/products/339);
(Note: Of course you will need a LyPo charger like the http://www.sparkfun.com/products/12711 but I didn't included it here, since it is not attached to the device, what means the battery should be charged before connecting it to the LiPower module.)
- 1 x 3-axis Compass and Accelerometer combined I2C sensor - for example, I used the very handy Pololu board LSM303DLH 3D Carrier with Voltage Regulators (http://www.pololu.com/product/1250);
- 1 x Bluetooth to Serial Port Module - I used the "Módulo Bluetooth para microcontroladores" board from Tato (http://www.tato.ind.br/produto/M%F3dulo-Bluetooth-para-microcontroladores.html - in Portuguese) which is based on the iTead Studio HC-05 board (http://imall.iteadstudio.com/im120723009.html) adapted for a 5V power supply and other convenient features;
- 4 x standard 3.5mm female audio jacks to plug the external buttons (mono or stereo, depending on what you want to connect; for example: if your button requires power, use the stereo conector, otherwise the mono will work);
- 4 x standard 3.5mm male audio jacks to connect the external buttons (mono or stereo, just like above);
- 4 x external buttons (be creative, that's why the buttons are external from the device ;-).

You will also need an FTDI Basic Breakout board (e.g. https://www.sparkfun.com/products/9716) and USB cable to load the software into your LilyPad Arduino.

Hookup Guide
------------

In a very quick and simplified way, everything should be connected like this:

1) Connect the LilyPad RGB LED module:

		Arduino      Output
		-------------------
		     5V  ->  + petal
		     11  ->  R petal
		      9  ->  G petal
		     10  ->  B petal

2) Connect the LilyPad Buzzer:

		Arduino      Output
		-------------------
		     13  ->  + petal
		    GND  ->  - petal

3) Connect the LilyPad Vibe Board:

		Arduino      Output
		-------------------
		      8  ->  + petal
		    GND  ->  - petal

4) Connect the Compass and Accelerometer board (of course, it will depend on the board you choose to use):

		Arduino      LSM303 board
		-------------------------
		     5V  ->  VIN
		    GND  ->  GND
		     A4  ->  SDA
		     A5  ->  SCL

5) Connect the Bluetooth to Serial Port Module (of course, it will depend on the board you choose to use):
	 
		           Arduino      Tato's Bluetooth board
		           -----------------------------------
		                5V  ->  VCC
		               GND  ->  GND
		Digital pin 0 (Rx)  ->  Tx
		Digital pin 1 (Tx)  ->  Rx
	 
6) Connect the 3.5mm female audio jacks for the input buttons to the Arduino digital input pins:

		      Arduino      Female jack 
		      ------------------------
		Digital pin 2  ->  the tip of the audio jack for button 1
		Digital pin 3  ->  the tip of the audio jack for button 2
		Digital pin 4  ->  the tip of the audio jack for button 3
		Digital pin 5  ->  the tip of the audio jack for button 4
		          GND  ->  the sleeve of all other tips of all the above jacks
		           5V  ->  the middle ring of the above jacks
	 
7) External buttons:

		Hook the male audio jacks to each one of the buttons you choose to use so you can
		plug them into the female jacks above.
	 
External dependencies
---------------------

You will need the following Arduino libraries:
- Kalman.h: A simplified one dimensional Kalman filter implementation for Arduino --> http://github.com/bachagas/Kalman
- lsm303-arduino: Arduino library for Pololu LSM303 boards --> http://github.com/bachagas/lsm303-arduino 

Installation
------------

Download the archive from GitHub, decompress it, and use it :)
Make sure you have all the libraries above correctly installed in your Arduino development environment.

Other information
-----------------

Nothing else to say for now ;-)
