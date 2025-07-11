#include <AFMotor.h>
#include <Servo.h>
#include <Arduino.h>

Servo clampServo;

const int SERVO_PIN = 46; //the only working pin that is not covered by motor shield ;-;

int clampAngle = 0; //start unclamped

static bool clamped = false;

AF_DCMotor rightMotorback(1);
AF_DCMotor rightMotorfront(2);
AF_DCMotor leftMotorback(3);
AF_DCMotor leftMotorfront(4);

/*
const byte BUFFER_SIZE = 16;
char buffer[BUFFER_SIZE];
byte bufferIndex = 0;
*/

void setup() {
  Serial.begin(115200);
  clampServo.attach(SERVO_PIN);
  Serial.println("Robot activating...");
  delay(500);
  Serial.println("Robot ready!");
  clampAngle = 180;
}

void loop() {
  if (Serial.available()) {
    // Expecting args like: "120,80,50"
    String input = Serial.readStringUntil('\n');
    int commaIndex1 = input.indexOf(',');
    int commaIndex2 = input.indexOf(',', commaIndex1 + 1);
    if (commaIndex1 > 0) {
      int leftSpeed = input.substring(0, commaIndex1).toInt();
      int rightSpeed = input.substring(commaIndex1 + 1, commaIndex2).toInt();
      int clampAngle = input.substring(commaIndex2+1).toInt();
      
      Serial.print("Left wheel speed: "); Serial.print(leftSpeed); Serial.print(" | Right wheel speed: "); Serial.println(rightSpeed);

      move(leftSpeed, rightSpeed);
      if (clampAngle) clampServo.write(clampAngle); else clampServo.write(0);
    }
  }    
}

void move(int leftSpeed, int rightSpeed) {
  leftMotorback.setSpeed(leftSpeed);
  leftMotorfront.setSpeed(leftSpeed);
  rightMotorback.setSpeed(rightSpeed);
  rightMotorfront.setSpeed(rightSpeed);

  leftMotorfront.run(FORWARD);
  leftMotorback.run(FORWARD);
  rightMotorfront.run(FORWARD);
  rightMotorback.run(FORWARD);
}

/*
void parseCommand(char* cmd) {
  // Expecting args like: "120,80"
  int commaIndex = -1;
  for (byte i = 0; i < BUFFER_SIZE; i++) {
    if (cmd[i] == ',') {
      commaIndex = i;
      break;
    }
  }

  if (commaIndex > 0) {
    cmd[commaIndex] = '\0';  // Split into two substrings
    int leftSpeed = atoi(cmd);
    int rightSpeed = atoi(cmd + commaIndex + 1);

    leftSpeed = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    //still using tank style turn
  }
}   

*/