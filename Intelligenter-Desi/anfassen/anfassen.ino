#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <MFRC522.h>

// -------------------- Ultraschall --------------------
class UltrasonicSensor {
  int trigPin, echoPin;

public:
  UltrasonicSensor(int trig, int echo) {
    trigPin = trig;
    echoPin = echo;
  }

  void begin() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  }

  long getDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    long distance = duration * 0.034 / 2;
    return distance;
  }
};

// -------------------- Pumpe --------------------
class Pump {
  int pin;
  bool active = false;
  unsigned long startTime = 0;
  unsigned long duration = 1000;

public:
  Pump(int p) { pin = p; }

  void begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  void activate() {
    digitalWrite(pin, HIGH);
    Serial.println("Pumpe AN");
    delay(1000);
    digitalWrite(pin, LOW);
    Serial.println("Pumpe AUS");
  }

 
};

// -------------------- RTC --------------------
class RTCModule {
  RTC_DS3231 rtc;

public:
  void begin() { rtc.begin();
}

  String getTime() {
    
    DateTime now = rtc.now();
    int hourDE = now.hour() + 1; 
    if (hourDE >= 24) hourDE -= 24;
    String t = "";
    t += String(now.day()) + ".";
    t += String(now.month()) + ".";
    t += String(now.year()) + " ";
    t += String(now.hour()) + ":";
    t += String(now.minute()) + ":";
    t += String(now.second());
    return t;
  }
};

// -------------------- RFID --------------------
class RFIDModule {
public:
  MFRC522 rfid;

  RFIDModule(int ss, int rst) : rfid(ss, rst) {}

  void begin() {
    SPI.begin();
    rfid.PCD_Init();
  }

  bool isNewCardPresent() {
    return rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();
  }

  String getUID() {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) uid += "0"; // führende Null
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
  }
};

// -------------------- OBJEKTE --------------------
UltrasonicSensor sensor(7, 6);
Pump pump(5);
RTCModule rtcModule;
RFIDModule rfidModule(10, 9);

// -------------------- VARIABLEN --------------------
long threshold = 10;
String currentUID = "";
bool cardPresent = false;

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);
  sensor.begin();
  pump.begin();
  rtcModule.begin();
  rfidModule.begin();
  Serial.println("System gestartet...");
}

// -------------------- LOOP --------------------
void loop() {
  long distance = sensor.getDistance();
  Serial.println("Distanz: " + String(distance) + " cm");

  // -------- RFID prüfen --------
  if (rfidModule.isNewCardPresent()) {
    currentUID = rfidModule.getUID();
    cardPresent = true;
  }

  // -------- Pumpen-Logik --------
  if (distance > 0 && distance < threshold) {
    String timeDE = rtcModule.getTime();
    if (cardPresent) {
      Serial.println("Personal erkannt: " + currentUID);
      Serial.println("Zeit: " + timeDE);
    } else {
      Serial.println("Patient erkannt");
    }

    pump.activate();
    

  // -------- Karte weg? --------
  if (!rfidModule.rfid.PICC_IsNewCardPresent() && cardPresent) {
    cardPresent = false;
    currentUID = "";
  }

  delay(4000); // kurze Pause für Loop
}
}
