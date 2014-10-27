aCap
====

An electronic cap based on the LilyPad Arduino for detecting and transmitting head movements to emulate a computer mouse (and do some other stuff ;-).

aCap was originally developed to be used as an assistive technology device for people with severe physical disabilities, like tetraplegia.

![aCap](http://nutap.les.inf.puc-rio.br/wp-content/uploads/2014/09/Featured-image.png)

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
		 0 (Rx)  ->  Tx
		 1 (Tx)  ->  Rx
	 
6) Connect the 3.5mm female audio jacks for the input buttons to the Arduino digital input pins:

		Arduino      Female jack 
		------------------------
		      2  ->  the tip of the audio jack for button 1
		      3  ->  the tip of the audio jack for button 2
		      4  ->  the tip of the audio jack for button 3
		      5  ->  the tip of the audio jack for button 4
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

Output
------

Detectable movements will be:
- Roll (bend head left-right): from accelerometer y-axis
- Pitch (bend head forwards-backwards): from accelerometer x-axis
- Nod / yaw (turn head left-right: from compass x-axis

When activated, the device will send through Bluetooth an output containing:

- "*" --> stands for comment lines, for example, the device will print that immediately after activation:

		*** Sensitivity is: 100
		*** Filter parameters are: 0.09800,91.00000,0.00000 - 0.09800,91.00000,0.00000 - 0.09800,91.00000,0.00000
		*** Reference values are: -1419,2729,-30
		                                 |__ stands for the initial sensor values that will be used as reference values when detecting relative moves: Nod, Pitch, Roll respectively

- a 7-tuple comma separated string like this:

		0,0,0,0,0,0,0
		| | | | | | |__ Button 4 state: 1 pressed, 0 released
		| | | | | |____ Button 3 state: 1 pressed, 0 released
		| | | | |______ Button 2 state: 1 pressed, 0 released
		| | | |________ Button 1 state: 1 pressed, 0 released
		| | |__________ Nod move (relative to reference value): left + <-- 0 --> - right
		| |____________ Pitch move (relative to reference value): backwards (up) - <-- 0 --> + front (down)
		|______________ Roll move (relative to reference value): left + <-- 0 --> - right

Example:

		*** Reference values are: -1387,2782,-25
		*** Sensitivity is: 100
		*** Filter parameters are: 0.09800,91.00000,0.00000 - 0.09800,91.00000,0.00000 - 0.09800,91.00000,0.00000
		*** Reference values are: -1419,2729,-30
		0,0,0,0,0,0,0
		0,0,0,0,0,0,0
		0,1,0,0,0,0,0
		0,0,0,0,0,0,0
		-1,0,0,0,0,0,0
		0,1,0,0,0,0,0
		-1,1,0,0,0,0,0
		-1,3,0,0,0,0,0
		1,3,1,0,0,0,0
		-2,3,1,0,0,0,0
		-3,6,1,0,0,0,0
		-1,7,1,0,0,0,0



Other information
-----------------

Nothing else to say for now ;-)

License
-------

aCap hardware and software are available under the Apache 2.0 license.

Copyright 2014 Bruno Azevedo Chagas

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
