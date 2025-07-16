/*
  The following treats the Arduino as a mere dumb motor driver slave that simply complies with the commands received from the Pi
  The command handling has been kept pretty simple for testing purposes to get down the transmission and systems integration
  The previous code had a useless LCD attached that increased the command processing latency to 30 seconds because of lcd.print

  The camera needs to ping and send details once every 30 frames only, otherwise the hardware gets overwhelmed with the barrage of commands and we make no progress.

  ======================= The PID needs to be tuned more before it's usable, it's speed corrections are horrendous rn =======================
*/
#include <AFMotor.h>
#include <Servo.h>
#include <NewPing.h>

// === Debug flag ===
#define DEBUG false   // <<<<< Toggle debug messages

Servo cameraServo;

// Initial angles for servos -- pls forgive the naming scheme i was using the servos for a rotating camera before
int cameraAngle = 0;

// === Motors ===
int DRIVE_SPEED = 230;
const int MIN_SPEED = 100; // Minimum speed to avoid stalling
const int MAX_SPEED = 230;
#define TURN_SPEED 100

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// === Ultrasonic ===
#define TRIG_PIN 22
#define ECHO_PIN 23

// === State ===
bool clamped = false;
bool driving = false;
bool forward = true;
bool turningInPlace = false;
bool turnLeftInPlace = false;
bool turnRightInPlace = false;

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;  // ms

// === Smoothed distance ===
float smoothedDistance = -1;
const float SMOOTH_ALPHA = 0.7;  // Low-pass filter factor

void setup() {
  Serial.begin(115200);
  stopMotors();
  cameraServo.attach(46); //  servo connected to pin 46, pin 44 isn't working, 45 is spotty at best...
  cameraServo.write(cameraAngle);

  unclamp();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Bang-bang + ultrasonic ready!");
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

  float distance = readDistanceCM();
  if (distance > 0) {
    // Smooth distance with low-pass filter
    if (smoothedDistance < 0) smoothedDistance = distance;
    smoothedDistance = SMOOTH_ALPHA * smoothedDistance + (1 - SMOOTH_ALPHA) * distance;

    if (DEBUG) {
      Serial.print("Raw Distance: "); Serial.print(distance);
      Serial.print(" | Smoothed: "); Serial.println(smoothedDistance);
    }

    if (smoothedDistance <= 30.0 && driving && forward) {
      float scale = (smoothedDistance - 5.0) / 25.0;
      scale = constrain(scale, 0, 1);
      DRIVE_SPEED = MIN_SPEED + scale * (MAX_SPEED - MIN_SPEED);
    } else {
      DRIVE_SPEED = MAX_SPEED;
    }

  } else {
    // Ignore invalid reading; keep previous smoothedDistance
    if (DEBUG) Serial.println("Invalid distance (-1)");
  }

  if (smoothedDistance > 0 && smoothedDistance <= 5.0 && !clamped) {
    if (DEBUG) {
      Serial.print("Object close! Smoothed: ");
      Serial.println(smoothedDistance);
    }
    delay(300);  // small pause for stability
    clamp();
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

    case 'U':
      clamp();
      cameraAngle += 30;
      if (DEBUG) Serial.print("Camera angle: "), Serial.println(cameraAngle);
      break;

    case 'N':
      unclamp();
      cameraAngle -= 30;
      if (DEBUG) Serial.print("Camera angle: "), Serial.println(cameraAngle);
      break;
  }
}

void applyDrive() {
  int speed = DRIVE_SPEED;
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  if (DEBUG) {
    Serial.print("Drive Speed: ");
    Serial.println(speed);
  }

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

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

void clamp() {
  cameraServo.write(120);
  clamped = true;
  Serial.println("Clamped");
}

void unclamp() {
  cameraServo.write(170);
  clamped = false;
  Serial.println("Unclamped");
}
/*
  Note: The AFMotor library is used for motor control, and the Servo library is used for the camera servo.
  Ensure that the AFMotor library is compatible with your hardware setup.
*/