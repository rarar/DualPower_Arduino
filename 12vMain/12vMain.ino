#include <Adafruit_MotorShield.h>

#include <AFMotor.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);

unsigned long previousMillis = 0;
const long interval = 12;

float t = 0.0; // time in our sine function
float frequency = 0.01; // speed of oscillation
int minBrightness = 0;
int maxBrightness = 180;

void setup() {
  Serial.begin(9600);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int brightness1 = (sin(t) + 1) / 2 * (maxBrightness - minBrightness) + minBrightness; // map [-1, 1] from sine to [minBrightness, maxBrightness]
    int brightness2 = (sin(t + PI/2) + 1) / 2 * (maxBrightness - minBrightness) + minBrightness; // shifted by PI/2 to be partially out of phase
    
    motor1.setSpeed(brightness1);
    motor2.setSpeed(brightness2);

    t += frequency;
    if (t > 2 * PI) { // reset to avoid precision issues
      t -= 2 * PI;
    }
  }
}
