/*
   a-Cap - an eletronic Cap based on the LilyPad Arduino for detecting and transmitting head movements to emulate a mouse
   Standard PS/2 mouse protocol based on: http://www.computer-engineering.org/ps2mouse/ 
   Typical mouse:
     Sample Rate = 100 samples/sec
     Resolution = 4 counts/mm
   Aproximately resolution for pitch and roll movements detected by an accelerometer:
     128 quids variation ~= 90º tilt in the respective axis (1 quid = the output from the 10-bit internal ADC, a number from 0-1023)
     Resolution = 1º / quid delta (typical mouse is 
 */

#include <Kalman.h>
#include <Wire.h>
#include <LSM303.h>

// Hardware input pins:
LSM303 compass; // Arduino library for Pololu LSM303 boards: http://www.pololu.com/catalog/product/2124
                // Library code from: https://github.com/pololu/lsm303-arduino
// Detectable movements will be:
// - roll move: from accelerometer y-axis
// - pitch move: from accelerometer x-axis
// - nod / yaw move: from compass x-axis
char report[80]; //for debugging purposes only

int btnPin1 = 2;       // input pin for the button 1
int btnPin2 = 3;       // input pin for the button 2
int btnPin3 = 4;       // input pin for the button 3
int btnPin4 = 5;       // input pin for the button 4

// Hardware output pins:
int redPin = 11;	// R petal on RGB LED module connected to digital pin 11
int greenPin = 9;	// G petal on RGB LED module connected to digital pin 9
int bluePin = 10;  	// B petal on RGB LED module connected to digital pin 10
int viberPin = 8;
int speakerPin = 13;

// Inputs variables:
int xValue,yValue,zValue,xRef,yRef,zRef,xDelta,yDelta,zDelta,x,y,z = 0;  // variables to store the values coming from the sensors and calculate displacements in the (x,y) axis
boolean btn1,btn2,btn3,btn4,btnPressed,oldBtnPressed = false;

// Sensor signal filter parameters:
/* The accelerometer raw delta will range from 0..128 quids (128 = ~90º - empirical)
   We can set the filter parameters as bellow:
   * importance of the process noise q:
   0 (stable processes) <-- 50 (mid changing) --> 128 (fast changing)
   * sensor noise r:
   1 (little noise) <-- 50 (moderate) --> 128 (very much noise)
   * estimation error p is not very important since it is adjusted (down)
     during the process. It must be just high enough to narrow down.
     We will initialize it with the middle value of 512
*/
//sugested initial values from: http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/
Kalman xFilter(0.125, 32, 100, 0);
Kalman yFilter(0.125, 32, 100, 0);
Kalman zFilter(0.125, 32, 100, 0);

// Control parameters: - TO BE REVIEWED / CLEANED
boolean active, oldActive = false;
int attenuation = 1; // will regulate the filter parameters: the greater the value, more sensible to the accelerometer raw inputs
int sensitivity = 1; // counts/quid - ?
int tolerance = 10; //for the sensitivity changes 10/1000 = 1%

unsigned long timer;

// LED alerts control variables:
boolean blinking = false;
unsigned long blinkTime = -1;
unsigned long blinkOn = 100;
unsigned long blinkOff = 2900;
unsigned long activeOff = 200;
unsigned long deactiveOff = 2900;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // initialize I2C LSM303 chip (compass + accelerometer):
  Wire.begin();
  compass.init();
  compass.enableDefault();
  
  // set pin modes:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(viberPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(btnPin1, INPUT_PULLUP);
  pinMode(btnPin2, INPUT_PULLUP);
  pinMode(btnPin3, INPUT_PULLUP);
  pinMode(btnPin4, INPUT_PULLUP);

  // initialize device parameters and control variables:
  sensitivity = 1; //empirical
  attenuation = 1; //empirical
  active = false; oldActive = true; //this will force calibration on the first loop
  Serial.println(); //"clean up" serial port
  calibrate();
  
  //just to show when turning on ;-)
  for (int i=0;i<2;i++){
    color(255,255,255);
    vibe(200); //digitalWrite(viberPin,HIGH);
    beep(700,400);
    delay(100);
  }
  color(0,0,0);
  digitalWrite(viberPin,LOW);
  noTone(speakerPin);
}

void loop() {
  timer = millis();
  
  // read the values from the sensors:
  compass.read();
  // for debug only:
  /*snprintf(report, sizeof(report), "A: %6d %6d %6d    M: %6d %6d %6d",
    compass.a.x, compass.a.y, compass.a.z,
    compass.m.x, compass.m.y, compass.m.z);
  Serial.println(report);*/

  xValue = compass.a.y; //roll move
  yValue = compass.a.x; //pitch move
  zValue = compass.m.x; //nod/yaw move
  
  //Buttons detection and control:
  btn1 = !digitalRead(btnPin1);
  btn2 = !digitalRead(btnPin2);
  btn3 = !digitalRead(btnPin3);
  btn4 = !digitalRead(btnPin4);
  //btnPressed = (btn1==LOW)||(btn2==LOW)||(btn3==LOW)||(btn4==LOW);
  btnPressed = (btn1||btn2||btn3||btn4);
  if (btnPressed!=oldBtnPressed){
    if (oldBtnPressed) {
      if (!active) activate();
      else deactivate();
    }
  }
  oldBtnPressed = btnPressed;
  
  //Calculates sensor values changes:
  xDelta = round((xValue - xRef)/attenuation);
  yDelta = round((yValue - yRef)/attenuation);
  zDelta = round((zValue - zRef)/attenuation);

  //x = xDelta;
  //y = yDelta;
  //z = zDelta;
  
  //Calculates filtered values and sends data by serial port:
  if (active) {
    x = round(xFilter.getFilteredValue((double)xDelta));
    y = round(yFilter.getFilteredValue((double)yDelta));
    z = round(zFilter.getFilteredValue((double)zDelta));
  
    printXYZ(x,y,z); Serial.print(",");
    printXYZ(btn1,btn2,btn3); Serial.print(","); Serial.println(btn4);
  }

  //Control the alerts
  showAlerts();
  
  oldActive = active;
  delay(10); // 100 samples/sec, just like a mouse
}

void calibrate() {
//Calibrates the initial reference values of the accelerometer and reset the filters parameters according to the sensitivity value
  long x,y,z;
  int i;
  
  x = 0; y = 0; z = 0;
  //Average value from the 10 last consucutive sensor reads:
  for (i=0;i<10;i++) {
    compass.read();
    //x += analogRead(xAxisPin);
    //y += analogRead(yAxisPin);
    //z += analogRead(zAxisPin);
    x += compass.a.y;
    y += compass.a.x;
    z += compass.m.x;
    delay(100);
  }
  xRef = round(x/i);
  yRef = round(y/i);
  zRef = round(z/i);
  
  Serial.print("*** Reference values are: ");
  printXYZ(xRef,yRef,zRef); Serial.println();
}

void setFilters(int sensitivity) {
  xFilter.setParameters((double)map(sensitivity, 0, 1023, 1, 1000)/1000,map(sensitivity, 0, 1023, 100, 1));
  yFilter.setParameters((double)map(sensitivity, 0, 1023, 1, 1000)/1000,map(sensitivity, 0, 1023, 100, 1));
  zFilter.setParameters((double)map(sensitivity, 0, 1023, 1, 1000)/1000,map(sensitivity, 0, 1023, 100, 1));

  Serial.print("*** Sensitivity is: "); Serial.print(sensitivity);
  Serial.print("\n*** Filter parameters are: ");
  Serial.print(xFilter.getProcessNoise(), 5); Serial.print(","); 
  Serial.print(xFilter.getSensorNoise(), 5); Serial.print(",");
  Serial.print(xFilter.getEstimatedError(), 5); Serial.print(" - ");
  Serial.print(yFilter.getProcessNoise(), 5); Serial.print(","); 
  Serial.print(yFilter.getSensorNoise(), 5); Serial.print(",");
  Serial.print(yFilter.getEstimatedError(), 5); Serial.print(" - ");
  Serial.print(zFilter.getProcessNoise(), 5); Serial.print(","); 
  Serial.print(zFilter.getSensorNoise(), 5); Serial.print(",");
  Serial.print(zFilter.getEstimatedError(), 5); Serial.println();
}

void printXYZ(int x, int y, int z) {
//Print a (x,y,z) integer tuple to the serial port
    Serial.print(x); Serial.print(",");
    Serial.print(y); Serial.print(",");
    Serial.print(z);
}

void color (unsigned char red, unsigned char green, unsigned char blue)	//the color generating function 
//Turns as RGB on with the respective RGB color
{	 
   analogWrite(redPin, 255-red);
   analogWrite(bluePin, 255-blue);
   analogWrite(greenPin, 255-green);
}

void showAlerts() {
    if (!blinking) {
      if ((timer - blinkTime) > blinkOff) {
        blinkTime = timer;
        blinking = true;
        if (active) color(0,255,0); //turn the RGB LED green
        else color(0,0,255); //turn the RGB LED blue
      }
    } else {
        if ((timer - blinkTime) > blinkOn) {
            blinking = false;
            blinkTime = timer;
            color(0,0,0); //turn the RGB LED off
        }
    }
}

void beep (int frequencyInHertz, long timeInMilliseconds)     // the sound producing function
{
          tone(speakerPin,(unsigned int)frequencyInHertz,(unsigned long)timeInMilliseconds);
          int pauseBetweenNotes = 120; //timeInMilliseconds * 0.30;
          // stop the tone playing:
          delay(pauseBetweenNotes);
          noTone(speakerPin);
}

void beep2 (int frequencyInHertz, long timeInMilliseconds)     // the sound producing function
{	 
          int x;	 
          long delayAmount = (long)(1000000/frequencyInHertz);
          long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
          for (x=0;x<loopTime;x++)	 
          {	 
              digitalWrite(speakerPin,HIGH);
              delayMicroseconds(delayAmount);
              digitalWrite(speakerPin,LOW);
              delayMicroseconds(delayAmount);
          }	 
}

void activate() {
          color(0,255,0);
          //digitalWrite(viberPin,HIGH);
          delay(200);
          vibe(200);
          active = true;
          // "Ta da" sound:
          beep(2349,100);	//D
          beep(4186,200);	//C
          
          // Perform device calibration:
          //sensitivity = analogRead(potentiometerPin);
          sensitivity = 100; //empirical
          //if (abs(sensitivity - oldSensitivity)>tolerance) {
          //if (active != oldActive) {
          setFilters(sensitivity);
          calibrate();
          //showAlert(1000);
          //}
          //oldSensitivity = sensitivity;
          
          // Turn everything off:
          color(0,0,0);
          digitalWrite(viberPin,LOW);
          noTone(speakerPin);
          blinkOff = activeOff;
}

void deactivate() {
          color(255,0,0);
          delay(200);
          //digitalWrite(viberPin,HIGH);
          vibe(200);
          active = false;
          // "Da ta" sound:
          beep(4186,100);	//C
          beep(1043,100);	//G
          beep(700,100);	//F
                    
          // Turn everything off:
          color(0,0,0);
          digitalWrite(viberPin,LOW);
          noTone(speakerPin);
          blinkOff = deactiveOff;
}

void vibe(long timeInMilliseconds) { // this is to attenuate to 50% the viber power
          int x;
          double power = 0.8; //20%
          int frequencyInHertz = 1000;
          long delayAmount = (long)(1000000/frequencyInHertz);
          long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
          long upTime = (long)2*delayAmount*power;
          long downTime = (long)2*delayAmount*(1-power);
          for (x=0;x<loopTime;x++)	 
          {	 
              digitalWrite(viberPin,HIGH);
              delayMicroseconds(upTime);
              digitalWrite(viberPin,LOW);
              delayMicroseconds(downTime);
          } 
}
