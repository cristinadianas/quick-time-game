#include <SPI.h>

volatile char comanda = '\0';
volatile bool comandaNoua = false;

// Pini butoane pentru fiecare culoare si fiecare jucator
const int pinButonRosuJucator1 = 4;
const int pinButonVerdeJucator1 = A0;
const int pinButonAlbastruJucator1 = 2;

const int pinBuzzer = 3;

const int pinButonRosuJucator2 = 5;
const int pinButonVerdeJucator2 = 6;
const int pinButonAlbastruJucator2 = 7;

// LED RGB (pinii pentru fiecare culoare) si fiecare jucator
const int pinLEDRosuJucator1 = 8;
const int pinLEDVerdeJucator1 = 9;
const int pinLEDAlbastruJucator1 = A2;

const int pinLEDRosuJucator2 = A5;
const int pinLEDVerdeJucator2 = A4;
const int pinLEDAlbastruJucator2 = A3;

unsigned long momentApasare = 0;
bool asteaptaApasare = false;
bool randJucator1 = true;

void setup() {
  Serial.begin(115200);
  SPCR |= bit(SPE);               // SPI in modul slave
  pinMode(MISO, OUTPUT);          // Seteaza MISO drept output pentru SPI
  SPI.attachInterrupt();
  
  pinMode(pinButonRosuJucator1, INPUT_PULLUP);
  pinMode(pinButonVerdeJucator1, INPUT_PULLUP);
  pinMode(pinButonAlbastruJucator1, INPUT_PULLUP);
  pinMode(pinButonRosuJucator2, INPUT_PULLUP);
  pinMode(pinButonVerdeJucator2, INPUT_PULLUP);
  pinMode(pinButonAlbastruJucator2, INPUT_PULLUP);

  pinMode(pinLEDRosuJucator1, OUTPUT);
  pinMode(pinLEDVerdeJucator1, OUTPUT);
  pinMode(pinLEDAlbastruJucator1, OUTPUT);
  pinMode(pinLEDRosuJucator2, OUTPUT);
  pinMode(pinLEDVerdeJucator2, OUTPUT);
  pinMode(pinLEDAlbastruJucator2, OUTPUT);
}


ISR(SPI_STC_vect) {
  char charPrimit = SPDR;
  if (charPrimit != '#') {
    comanda = charPrimit;
    comandaNoua = true;
  }
}


void loop() {
  if (comandaNoua) {
    comandaNoua = false;

    if (comanda == 'r' || comanda == 'g' || comanda == 'b') {
      startButon(comanda);  // Incepe o noua runda cu culoarea primita
    } else {
      SPDR = '$';  // Comanda nerecunoscuta
    }
  }

  if (asteaptaApasare) {
    verificaButonApasat();
  }
}


void startButon(char color) {
  Serial.print("Received: ");
  Serial.print((char)SPDR);
  Serial.println(randJucator1 ? " for p1" : " for p2");
  momentApasare = millis();
  asteaptaApasare = true;

  activateLED(color);  // Activeaza LED-ul pentru jucatorul curent in functie de culoarea primita
}


void verificaButonApasat() {
  unsigned long timpDeLaApasare = millis() - momentApasare;
  
  int pinButon;
  if (randJucator1) {
    pinButon = (comanda == 'r') ? pinButonRosuJucator1 :
                (comanda == 'g') ? pinButonVerdeJucator1 :
                pinButonAlbastruJucator1;
  } else {
    pinButon = (comanda == 'r') ? pinButonRosuJucator2 :
                (comanda == 'g') ? pinButonVerdeJucator2 :
                pinButonAlbastruJucator2;
  }

  // Verifica apasarea butonului
  if (digitalRead(pinButon) == LOW) {
    asteaptaApasare = false;
    char scor;

    if (timpDeLaApasare <= 300) {
      scor = 'a';  // Raspuns rapid
    } else if (timpDeLaApasare <= 600) {
      scor = 'b';  // Raspuns mediu
    } else if (timpDeLaApasare <= 1000) {
      scor = 'c';  // Rapuns incet
    } else {
      scor = 'i';  // Ratat
    }
    
    SPDR = scor;  // Trimite scorul prin SPI
    if(scor != 'i')
      tone(pinBuzzer, 659, 200);

    reseteazaLEDurile();
    randJucator1 = !randJucator1;  // Randul celuilalt jcator
  } else if (timpDeLaApasare > 900) {
    asteaptaApasare = false;
    SPDR = 'i';
    reseteazaLEDurile();
    randJucator1 = !randJucator1;  // Randul celuilalt jucator
  }
  
  if((randJucator1 && (digitalRead(pinButonAlbastruJucator1) == LOW || digitalRead(pinButonRosuJucator1) == LOW || digitalRead(pinButonVerdeJucator1) == LOW))
  ||
    (!randJucator1 && (digitalRead(pinButonAlbastruJucator2) == LOW || digitalRead(pinButonRosuJucator2) == LOW || digitalRead(pinButonVerdeJucator2) == LOW))) {
    
    tone(pinBuzzer, 220, 200);  // Buton apasat gresit
    reseteazaLEDurile();
  }
}


void activateLED(char color) {
  reseteazaLEDurile();
  int ledPin;
  
  if (randJucator1) {
    ledPin = (color == 'r') ? pinLEDRosuJucator1 :
             (color == 'g') ? pinLEDVerdeJucator1 :
             pinLEDAlbastruJucator1;
  } else {
    ledPin = (color == 'r') ? pinLEDRosuJucator2 :
             (color == 'g') ? pinLEDVerdeJucator2 :
             pinLEDAlbastruJucator2;
  }

  digitalWrite(ledPin, HIGH);
}


void reseteazaLEDurile() {
  digitalWrite(pinLEDRosuJucator1, LOW);
  digitalWrite(pinLEDVerdeJucator1, LOW);
  digitalWrite(pinLEDAlbastruJucator1, LOW);
  digitalWrite(pinLEDRosuJucator2, LOW);
  digitalWrite(pinLEDVerdeJucator2, LOW);
  digitalWrite(pinLEDAlbastruJucator2, LOW);
}
