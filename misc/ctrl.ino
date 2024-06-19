#include <SBUS.h>
#include <limits.h>
#include <Servo.h> // remove 
#include "HCPCA9685.h"

#define PCA_ADDRESS 0x40  // to be set

HCPCA9685 sm(PCA_ADDRESS);

// Remember to unplug the RX pin of your UNO when uploading this sketch.
SBUS sbus(Serial1);
// This is timer2, which triggers every 1ms and processes the incoming SBUS datastream.
ISR(TIMER2_COMPA_vect) {
  sbus.process();
}

int data[10];
int buttonStates[10];
// SERVO Angles in I(initial), O(open), C(close) positions
int s[10][3] = {
  { 160, 25, 120 },
  { 25, 25, 125 },
  { 25, 25, 140 },
  { 0, 0, 115 },
  { 0, 152, 125 },
  { 160, 70, 125 },
  { 25, 25, 130 },
  { 55, 55, 165 },
  { 20, 20, 120 },
  { 20, 90, 120 },
};


void close00() {  // close arr1, means servo 0 to 4
  for (int j = 0; j <= 4; j++) {
    sm.Servo(j, s[j][2]);  // close
  }
}
void close02() {  // close arr2, means servo 5 to 9
  for (int j = 5; j <= 9; j++) {
    sm.Servo(j, s[j][2]);  // close
  }
}
void open10() {  // open arr1 ke ends , servo 0,1 and 3,4
  sm.Servo(0, s[0][1]);
  sm.Servo(1, s[1][1]);
  sm.Servo(3, s[3][1]);
  sm.Servo(4, s[4][1]);
}
void open11() {  // open middle wala gripper, servo 2,7
  sm.Servo(2, s[2][1]);
  sm.Servo(7, s[7][1]);
}
void open12() {  // open arr2 ends , servo 5,6 and 8,9
  sm.Servo(5, s[5][1]);
  sm.Servo(6, s[6][1]);
  sm.Servo(8, s[8][1]);
  sm.Servo(9, s[9][1]);
}



void initialServoPos() {
  for (int i = 0; i < 10; i++) {  // total 10 gripper servos
    sm.Servo(i, s[i][0]);
  }
}

void setup() {
  Serial.begin(115200);
  sm.Init(SERVO_MODE);
  sm.Sleep(false);
  initialServoPos();  // set initial servo Position
  sbus.begin();
}

void readChannel() {
  for (int i = 0; i < 10; ++i) {
    data[i] = sbus.getChannel(i + 1);
  }
  delay(10);
}

/* 
  @param switchDv , either 0(close gripper) or 1(open gripper)
  @param switchAv , for Dv = 0 close: ( 0 - arr1 or 2 -arr2)
                    for Dv = 1 open:  (0 - arr1_ends or 1 - middle grippers or 2 - arr2_ends)
*/

void servoTrigger(int switchDv, int switchAv) {
  if (switchDv == 0) {  // close
    switch (switchAv) {
      case 0:  // close arr1, means servo 0 to 4
        close00();
        Serial.println("close ---- case0");
        break;
      case 2:  // close arr2, means servo 5 to 9
        close02();
        Serial.println("close ---- case2");
        break;
    }
  } else if (switchDv == 1) {  // open
    switch (switchAv) {
      case 0:
        open10();
        Serial.println("open ---- case0");
        break;
      case 1:
        open11();
        Serial.println("open ---- case1");
        break;
      case 2:
        open12();
        Serial.println("open ---- case2");
        break;
    }
  }
}

void switchState() {
  for (int i = 4; i <= 8; i++) {
    if (data[i] == 172) {
      buttonState[i] = 0;
    } else if (data[i] == 992) {
      buttonState[i] = 1;
    } else if (data[i] == 1811) {
      buttonState[i] == 2;
    }
  }
  // map data from potentiometer to control the height of grippers, data[9]
  // trigger set SB
  buttonState[9] = map(data[9], 172,1811 , 0 ,90); // mapped the data in angles from 0 to 90 degrees
}


void mapSwitchData() {

  if (data[8] == 1811) {               // channel 9 trigger SE
    int sDv = data[7] == 172 ? 0 : 1;  // channel 8 SD
    int sAv;
    if (data[4] == 172) {  //channel  5 SA
      sAv = 0;
    } else if (data[4] == 992) {
      sAv = 1;
    } else if (data[4] == 1811) {
      sAv = 2;
    }
  }
  servoTrigger();  // call servo trigger
}


void loop() {
  readChannel();
  switchState();
  Serial.println("/*------Data-------*/");
  for (int i = 0; i < 10; ++i) {
    Serial.print("CH0");
    Serial.print(i + 1);
    Serial.print(" :   ");
    Serial.println(data[i]);
  }
  //
  mapSwitchData();
}
