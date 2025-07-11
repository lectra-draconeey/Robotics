/*
  The following treats the Arduino as a mere dumb motor driver slave that simply complies with the commands received from the Pi
  The command handling has been kept pretty simple for testing purposes to get down the transmission and systems integration
  The previous code had a useless LCD attached that increased the command processing latency to 30 seconds because of lcd.print

  The camera needs to ping and send details once every 30 frames only, otherwise the hardware gets overwhelmed with the barrage of commands and we make no progress.

  ======================= The PID needs to be tuned more before it's usable, it's speed corrections are horrendous rn =======================
*/
#include <AFMotor.h>
#include <Servo.h>

Servo cameraServo;

// Initial angles for servos -- pls forgive the naming scheme i was using the servos for a rotating camera before
int cameraAngle = 0;

// === Motors ===
#define DRIVE_SPEED 230
#define TURN_SPEED 180

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// === State ===
bool driving = false;
bool forward = true;
bool turningInPlace = false;
bool turnLeftInPlace = false;
bool turnRightInPlace = false;

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;  // ms

void setup() {
  Serial.begin(115200);
  stopMotors();
  cameraServo.attach(46); //  servo connected to pin 46, pin 44 isn't working, 45 is spotty at best...
  cameraServo.write(cameraAngle);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    char cmd = input.charAt(0);
    handleCommand(cmd);
    lastCommandTime = millis();
  }

  if (turningInPlace) {
    applyInPlaceTurn();
  } else if (driving) {
    applyDrive();
  } else {
    stopMotors();
  }

  if (millis() - lastCommandTime > COMMAND_TIMEOUT) {
    driving = false;
    turningInPlace = false;
    stopMotors();
  }
}

void handleCommand(char cmd) {
  switch (cmd) {
    case 'W':
      driving = true;
      forward = true;
      turningInPlace = false;
      break;

    case 'S':
      driving = true;
      forward = false;
      turningInPlace = false;
      break;

    case 'A':
      driving = false;
      turningInPlace = true;
      turnLeftInPlace = false;
      turnRightInPlace = true;
      break;

    case 'D':
      driving = false;
      turningInPlace = true;
      turnRightInPlace = false;
      turnLeftInPlace = true;
      break;

    case 'X':
      driving = false;
      turningInPlace = false;
      stopMotors();
      break;

    case 'U': // Camera Tilt Up
      cameraAngle += 30;
      if (cameraAngle > 180) cameraAngle = 180; // Constrain angle
      cameraServo.write(cameraAngle);
      Serial.print("Camera angle: "); Serial.println(cameraAngle);
      break;

    case 'N': // Camera Tilt Down
      cameraAngle -= 30;
      if (cameraAngle < 0) cameraAngle = 0; // Constrain angle
      cameraServo.write(cameraAngle);
      Serial.print("Camera angle: "); Serial.println(cameraAngle);
      break;  
  }
}

void applyDrive() {
  int speed = DRIVE_SPEED;

  motor1.setSpeed(speed);
  motor2.setSpeed(speed);
  motor3.setSpeed(speed);
  motor4.setSpeed(speed);

  if (forward) {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  } else {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }
}

void applyInPlaceTurn() {
  int speed = TURN_SPEED;

  motor1.setSpeed(speed);
  motor2.setSpeed(speed);
  motor3.setSpeed(speed);
  motor4.setSpeed(speed);

  if (turnLeftInPlace) {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  } else if (turnRightInPlace) {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }
}

void stopMotors() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}