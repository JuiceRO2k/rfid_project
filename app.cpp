#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>           // For SPI communication
#include <MFRC522.h>       // RFID library
#include <Servo.h>         // Servo motor library

#define SS_PIN 10          // SDA pin of RFID module
#define RST_PIN 9          // RST pin of RFID module
#define SERVO_PIN 6        // Servo motor pin
#define LED 7

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize LCD display
MFRC522 rfid(SS_PIN, RST_PIN);  // Initialize RFID module
Servo myServo;  // Initialize Servo motor

// Store the UID of the authorized RFID tag (replace with your own tag's UID)
byte authorizedUID[] = {0xD3, 0x30, 0x89, 0x11}; // Example UID, replace with your own
int correctPasscode[] = {A3, A2, A1, A2};  // Passcode 1234 (buttons A0-A3)

// Passcode sequence (buttons A0, A1, A2, A3 correspond to digits 1, 2, 3, 4)
int passcode[] = {A0, A1, A2, A3}; // The passcode "1234"
int enteredPasscode[4];  // To store the entered passcode
int passcodeIndex = 0;  // Index to keep track of entered digits

void setup() {
  Serial.begin(9600);
  SPI.begin(); rfid.PCD_Init();
  myServo.attach(SERVO_PIN); myServo.write(0); 

  lcd.init(); lcd.backlight(); lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Scan RFID or");
  lcd.setCursor(0, 1); lcd.print("Passcode");
  pinMode(LED, OUTPUT); digitalWrite(LED, LOW);

  for (int i = 0; i < 4; i++) {
    pinMode(passcode[i], INPUT);
  }
}

void loop() {
  if ( !rfid.PICC_IsNewCardPresent()) { checkPasskey(); return; }
  if ( !rfid.PICC_ReadCardSerial()) { return; }

  Serial.println("Card detected!");
  Serial.print("Scanned UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (checkUID(rfid.uid.uidByte, authorizedUID)) {
    lcd.clear(); lcd.print("Access Granted!"); digitalWrite(LED, HIGH);
    Serial.println("Access Granted - Door Unlocked!");
    myServo.write(90);  
    delay(5000);        
    myServo.write(0); lcd.clear();
    lcd.print("Door Locked!"); digitalWrite(LED, LOW);
    delay(1000);
    lcd.clear(); lcd.print("Scan RFID or");
    lcd.setCursor(0, 1); lcd.print("Passcode");
    passcodeIndex = 0;  
  } else {
    passcodeIndex = 0;  
    lcd.clear(); lcd.print("Access Denied!");
    delay(1000);
    lcd.clear(); lcd.print("Scan RFID or");
    lcd.setCursor(0, 1); lcd.print("Passcode");
  }

  rfid.PICC_HaltA();    // Stop reading
}

void checkPasskey() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(passcode[i]) == HIGH) {
      enteredPasscode[passcodeIndex] = passcode[i];
      passcodeIndex++;

      // Display the entered key on LCD
      lcd.setCursor(0, 1); lcd.print("Passcode: ");
      for (int j = 0; j < passcodeIndex; j++) {
        lcd.print(enteredPasscode[j] == A0 ? "1" : enteredPasscode[j] == A1 ? "2" : enteredPasscode[j] == A2 ? "3" : "4");
      }

      delay(500);  // Debounce delay
    }
  }

  if (passcodeIndex == 4) {
    if (checkPasscode()) {
      lcd.clear(); lcd.print("Access Granted!"); digitalWrite(LED, HIGH);
      Serial.println("Access Granted - Door Unlocked!");
      myServo.write(90);  
      delay(5000);        
      myServo.write(0);   
      lcd.clear(); lcd.print("Door Locked!"); digitalWrite(LED, LOW);
      delay(1000);
      lcd.clear(); lcd.print("Scan RFID or");
      lcd.setCursor(0, 1); lcd.print("Passcode");
    } else {
      lcd.clear(); lcd.print("Wrong Passcode!");
      delay(1000);
      lcd.clear(); lcd.print("Scan RFID or");
      lcd.setCursor(0, 1); lcd.print("Passcode");
    }
    passcodeIndex = 0;  // Reset the entered passcode
  }
}

// Function to compare UIDs
bool checkUID(byte *scannedUID, byte *authorizedUID) {
  for (byte i = 0; i < 4; i++) {
    if (scannedUID[i] != authorizedUID[i]) {
      return false; // UIDs -> no match
    }
  }
  return true; // UIDs -> match
}

// Function to check if the entered passcode is correct
bool checkPasscode() {
  for (int i = 0; i < 4; i++) {
    if (enteredPasscode[i] != correctPasscode[i]) {
      return false;  // Passcode -> incorrect
    }
  }
  return true;  // Passcode -> correct
}
