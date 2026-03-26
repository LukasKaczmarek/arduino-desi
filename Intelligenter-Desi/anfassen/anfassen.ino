#include <SPI.h>
#include <MFRC522.h>

const float soundSpeedCm = 0.034;
const int timeFor15ml = 3000;
const int handThreshold = 15;

class UltrasonicSensor {
  private:
    int trigPin;
    int echoPin;
    bool movement = false;
    int distance = 0;

  public:
    UltrasonicSensor(int trig, int echo){
      trigPin = trig;
      echoPin = echo;
    }

    bool getMovement() {
      return movement;
    }

    int getTrigPin() {
      return trigPin;
    }

    void setMovement(bool value) {
      movement = value;
    }

    void start(){
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
      digitalWrite(trigPin, LOW);
    }

    int measureDistance() {
      long duration;
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);
      int dist = duration * soundSpeedCm / 2;
      return dist;
    }

    bool isHandDetected() {
      float distance = measureDistance();
      Serial.print("Distance: ");
      Serial.println(distance);
      return (distance > 0 && distance <= handThreshold);
    }
};

class Waterpump {
  private:
    int pin;

  public:
    Waterpump(int p){
      pin = p;
    }

    void start(){
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

    void turnOn() {
      digitalWrite(pin, HIGH);
    }

    void turnOff() {
      digitalWrite(pin, LOW);
    }

    void pulse(int ms){
      turnOn();
      delay(ms);
      turnOff();
    }
};

class RFIDReader {
  private:
  MFRC522 mfrc;

  public:
  RFIDReader(int ssPin, int rstPin) : mfrc(ssPin, rstPin){}

  void start (){
    SPI.begin();
    mfrc.PCD_Init();
    Serial.println("RFID bereit");
  }
  long readI(){
    if(!mfrc.PICC_IsNewCardPresent()){
      return 0;
    }
    if(!mfrc.PICC_ReadCardSerial()){
      return 0;
    }
  }
}

UltrasonicSensor usSensor(9, 10);
Waterpump pump(8);

void setup() {
  Serial.begin(9600);
  usSensor.start();
  pump.start();
  Serial.println("Start");
}

void loop() {
   if (usSensor.isHandDetected()) {
    Serial.println("Hand erkannt! Pumpe läuft...");
    pump.pulse(timeFor15ml);
    delay(3000);
  } else {
    Serial.println("Keine Hand");
  }
}