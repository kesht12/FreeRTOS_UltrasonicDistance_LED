#include <HCSR04.h> //gamegine

HCSR04 hc(10, 46); // (trig pin, echo pin)

void setup(){
  Serial.begin (9600);
}
void loop(){
  delay(1000);
  Serial.println(hc.dist());

}