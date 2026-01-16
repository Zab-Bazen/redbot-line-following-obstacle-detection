#include <RedBot.h>
#include <SharpIR.h>

// -------------------- Pin / Sensor Configuration --------------------
// NOTE: Do not share an analog pin between two sensors.
// Center line sensor was on A6 in your original code, so IR must be moved.
#define IR_PIN A2
#define IR_MODEL 1080  // GP2Y0A21Y

#define LINE_LEFT_PIN   A3
#define LINE_CENTER_PIN A6
#define LINE_RIGHT_PIN  A7

RedBotSensor leftSensor(LINE_LEFT_PIN);
RedBotSensor centerSensor(LINE_CENTER_PIN);
RedBotSensor rightSensor(LINE_RIGHT_PIN);

SharpIR irSensor(IR_PIN, IR_MODEL);
RedBotMotors motors;

// -------------------- Tuning Parameters --------------------
#define LINE_THRESHOLD 900

#define BASE_SPEED 43     // 0-255
#define SHIFT      17

// Distance thresholds (cm). Make these non-overlapping.
#define STOP_DIST_CM   12   // stop if closer than this
#define AVOID_MIN_CM   12
#define AVOID_MAX_CM   25

// Avoidance behavior timing (ms)
#define AVOID_TURN_MS   500
#define AVOID_BRAKE_MS  250
#define AVOID_DRIVE_MS  1200

// -------------------- State Machine --------------------
enum RobotMode {
  MODE_LINE_FOLLOW = 0,
  MODE_AVOID_TURN,
  MODE_AVOID_BRAKE,
  MODE_AVOID_DRIVE,
  MODE_AVOID_STOP
};

RobotMode mode = MODE_LINE_FOLLOW;
unsigned long modeStartMs = 0;

// -------------------- Helpers --------------------
static inline int clampMotor(int v) {
  if (v > 255) return 255;
  if (v < -255) return -255;
  return v;
}

void setMotors(int left, int right) {
  motors.leftMotor(clampMotor(left));
  motors.rightMotor(clampMotor(right));
}

void enterMode(RobotMode newMode) {
  mode = newMode;
  modeStartMs = millis();
}

void setup() {
  Serial.begin(9600);
  Serial.println("RedBot: Line Following + Obstacle Avoidance");
  Serial.println("------------------------------------------");

  // Initialize speeds to safe values
  setMotors(0, 0);
  enterMode(MODE_LINE_FOLLOW);
}

void loop() {
  // ---- Read sensors once per loop ----
  const int leftVal   = leftSensor.read();
  const int centerVal = centerSensor.read();
  const int rightVal  = rightSensor.read();

  const unsigned long t0 = millis();
  const int distCm = irSensor.distance();
  const unsigned long irTimeMs = millis() - t0;

  // ---- Debug prints (optional) ----
  Serial.print("IR(cm)="); Serial.print(distCm);
  Serial.print(" IRt(ms)="); Serial.print(irTimeMs);
  Serial.print(" | L="); Serial.print(leftVal);
  Serial.print(" C="); Serial.print(centerVal);
  Serial.print(" R="); Serial.println(rightVal);

  // ---- Obstacle trigger logic (decide mode) ----
  // If something is very close, STOP immediately.
  if (distCm > 0 && distCm < STOP_DIST_CM) {
    motors.stop();
    enterMode(MODE_AVOID_STOP);
  }
  // If within avoidance window and we are following line, start avoidance sequence.
  else if (distCm >= AVOID_MIN_CM && distCm <= AVOID_MAX_CM && mode == MODE_LINE_FOLLOW) {
    motors.stop();
    enterMode(MODE_AVOID_TURN);
  }

  // ---- Mode behavior ----
  switch (mode) {
    case MODE_LINE_FOLLOW: {
      int leftSpeed = BASE_SPEED;
      int rightSpeed = BASE_SPEED;

      if (centerVal > LINE_THRESHOLD) {
        // On the line: go forward
        leftSpeed = BASE_SPEED;
        rightSpeed = BASE_SPEED;
      } else if (rightVal > LINE_THRESHOLD) {
        // Line is under right sensor: steer right
        leftSpeed = -(BASE_SPEED + SHIFT);
        rightSpeed = (BASE_SPEED - SHIFT);
      } else if (leftVal > LINE_THRESHOLD) {
        // Line is under left sensor: steer left
        leftSpeed = -(BASE_SPEED - SHIFT);
        rightSpeed = (BASE_SPEED + SHIFT);
      } else {
        // Lost line: slow forward or stop (choose behavior)
        leftSpeed = 0;
        rightSpeed = 0;
      }

      setMotors(leftSpeed, rightSpeed);
      break;
    }

    case MODE_AVOID_TURN: {
      // Turn in place (adjust if needed for your motor direction)
      setMotors(-110, -110);
      if (millis() - modeStartMs >= AVOID_TURN_MS) {
        motors.brake();
        enterMode(MODE_AVOID_BRAKE);
      }
      break;
    }

    case MODE_AVOID_BRAKE: {
      motors.brake();
      if (millis() - modeStartMs >= AVOID_BRAKE_MS) {
        enterMode(MODE_AVOID_DRIVE);
      }
      break;
    }

    case MODE_AVOID_DRIVE: {
      motors.drive(55);
      if (millis() - modeStartMs >= AVOID_DRIVE_MS) {
        motors.stop();
        enterMode(MODE_LINE_FOLLOW);
      }
      break;
    }

    case MODE_AVOID_STOP: {
      motors.stop();
      // Once obstacle is gone, resume
      if (distCm >= STOP_DIST_CM) {
        enterMode(MODE_LINE_FOLLOW);
      }
      break;
    }
  }

  // Small loop delay to reduce noise and serial spam
  delay(20);
}
