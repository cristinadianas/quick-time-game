#include <SPI.h>
#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);
Servo cronometruServo;

const int pinButonStart = 2;
const int pinServo = 9;
const int servoUnghiMaxim = 180;
const unsigned long timpTotalJoc = 30000;  // 30 secunde

char culori[] = {'r', 'g', 'b'};
unsigned long timpStartJoc = 0;
unsigned long timpUltimaRunda = 0;
unsigned long timpRunda = 1000;  // 1 secunda pe runda
bool jocInceput = false;
bool asteaptaRaspuns = false;
char raspuns;

String jucator1, jucator2;
int jucator1scor = 0;
int jucator2scor = 0;
int cronometruRunda = 0;
bool randJucator1 = true;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  pinMode(pinButonStart, INPUT_PULLUP);

  lcd.begin(16, 2);
  cronometruServo.attach(pinServo);

  lcd.print("Incepe jocul!");
}

void loop() {
  if (!jocInceput && digitalRead(pinButonStart) == LOW) {
    delay(200);
    jocInceput = true;
    lcd.clear();
    lcd.print("Nume jucator 1:");
    while (Serial.available() == 0) {}
    jucator1 = Serial.readStringUntil('\n'); 

    lcd.clear();
    lcd.print("Nume jucator 2:");
    while (Serial.available() == 0) {}
    jucator2 = Serial.readStringUntil('\n');

    timpStartJoc = millis();
  }

  if (jocInceput) {
    unsigned long millisCurent = millis();

    // Jocul se sfarseste daca au trecut 30 de secunde
    if (millisCurent - timpStartJoc >= timpTotalJoc) {
      jocInceput = false;
      displayWinner();
      resetGame();
      return;
    }

    // Roteste servo-ul
    int angle = map(millisCurent - timpStartJoc, 0, timpTotalJoc, 0, servoUnghiMaxim);
    cronometruServo.write(angle);

    // Randul celuilalt jucator dupa cate 1 secunda
    if (millisCurent - timpUltimaRunda >= timpRunda) {
      if (cronometruRunda < 30) {  // 15 runde fiecare jucator
        if (randJucator1) {
          lcd.clear();
          lcd.print("Runda " + jucator1);
          sendColorCommand();
        } else {
          lcd.clear();
          lcd.print("Runda " + jucator2);
          sendColorCommand();
        }
        delay(1000);
        asteaptaRaspuns = true;
        timpUltimaRunda = millisCurent;
        randJucator1 = !randJucator1;
        cronometruRunda++;
      }
    }

    // Primeste si proceseaza raspunsul de la slave
    if (asteaptaRaspuns) {
      raspuns = sendCommand('#');
      Serial.println((randJucator1 ? jucator2 : jucator1) + " scor: ");
      Serial.println(raspuns);

      scorNou(raspuns);
      asteaptaRaspuns = false;
    }
  }
}

// Functie care trimite culoarea prin SPI
void sendColorCommand() {
  char colorCommand = culori[random(0, 3)];
  sendCommand(colorCommand);
  Serial.print("Sent color: ");
  Serial.println(colorCommand);
}

// Noul scor in functie de raspunsul jucatorului
void scorNou(char scor) {
  int puncte = 0;
  switch (scor) {
    case 'a': puncte = 50; break;
    case 'b': puncte = 25; break;
    case 'c': puncte = 10; break;
    case 'i': puncte = 0; break;
  }

  if (!randJucator1) {
    jucator1scor += puncte;
  } else {
    jucator2scor += puncte;
  }
}

// Trimite comanda prin SPI si primeste raspuns
char sendCommand(char command) {
  digitalWrite(SS, LOW);
  char raspuns = SPI.transfer(command);
  digitalWrite(SS, HIGH);
  return raspuns;
}

// Afiseaza scorul final si castigatorul jocului
void displayWinner() {
  lcd.clear();
  lcd.print(jucator1 + " scor:");
  lcd.setCursor(0, 1);
  lcd.print(jucator1scor);
  delay(2000);

  lcd.clear();
  lcd.print(jucator2 + " scor:");
  lcd.setCursor(0, 1);
  lcd.print(jucator2scor);
  delay(2000);

  lcd.clear();
  if (jucator1scor > jucator2scor) {
    lcd.print("Castigator: " + jucator1);
  } else if (jucator2scor > jucator1scor) {
    lcd.print("Castigator: " + jucator2);
  } else {
    lcd.print("Egalitate!");
  }
  delay(2000);
}

// Reseteaza jocul la starea initiala
void resetGame() {
  jucator1scor = 0;
  jucator2scor = 0;
  cronometruRunda = 0;
  randJucator1 = true;
  lcd.clear();
  lcd.print("Incepe jocul!");
}
