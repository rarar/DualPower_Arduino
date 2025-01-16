//Libraries
#include <RBDdimmer.h>
//Parameters
const int zeroCrossPin = 2;
const int acdPin = 3;
int MIN_POWER = 20;
int MAX_POWER = 100;
int POWER_STEP = 2;
//Variables
int power = 0;
//Objects
dimmerLamp acd(acdPin);
void setup() {
  //Init Serial USB
  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  acd.begin(NORMAL_MODE, ON);
}
void loop() {
  testDimmer();
}
void testDimmer() {
  for (power = MIN_POWER; power <= MAX_POWER; power += POWER_STEP) {
    acd.setPower(power);
    Serial.print("lampValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(100);
  }

  for (power = MAX_POWER; power >= MIN_POWER; power -= POWER_STEP) {
    acd.setPower(power);
    Serial.print("lampValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(100);
  }
}
