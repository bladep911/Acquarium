#include <LiquidCrystal.h>
#include <OneWire.h>
#include <IRremote.h>
#include "pitches.h"

#define RECV_PIN 8
#define ON_BUTTON 0xFF629D
#define OFF_BUTTON 0xFFA25D

//Specifichiamo quali pin verranno utilizzati per gestire il Display specificandoli come di seguito LiquidCrystal(rs, rw, enable, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//Temperature chip i/o
int DS18S20_Pin = 7; //DS18S20 Signal pin on digital 7
int relayPin = 9; // Pin to control relay
OneWire ds(DS18S20_Pin); // on digital pin 7
IRrecv irrecv(RECV_PIN);
decode_results results;
bool autoFan;

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};


void setup() {
   playTone();
   Serial.begin(9600);
  //impostiamo il numero di colonne e di righe del nostro display 16x2
  irrecv.enableIRIn();
  
  lcd.begin(16, 2);
  lcd.print("Temperatura ");
  lcd.print((char)223);
  lcd.print("C:");
  autoFan = true;
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void playTone(){
for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void loop() {
  float temperature = getTemp();
  activeFan(temperature);
  Serial.println(temperature);
  //spostiamo il cursore all'inizio della seconda riga
  //ricordiamo che con 1 si indica la riga 2 perche' si inizia a contare da 0
  lcd.setCursor(0, 1);
  //visualizza i secondi dall'inizio dell'avvio di arduino
  lcd.print(temperature);
  delat(500);
}

void activeFan(int temp){
   if (irrecv.decode(&results)) {  //If IR receive results are detected
      Serial.println(results.value, HEX);
      switch (results.value) {
        //Power
        case ON_BUTTON:
           digitalWrite(relayPin, HIGH);
           autoFan = false;
           break;
        case OFF_BUTTON:
           digitalWrite(relayPin, LOW);
           autoFan = true;
           break;
      }
	  irrecv.resume();
  }
  
  if(autoFan){
    //check if manual mode
    int state = bitRead(PORTD,relayPin);
    if(temp >= 26 && state == LOW){
      digitalWrite(relayPin, HIGH);
    }
    
    if(temp <= 25.5 && state == HIGH){
      digitalWrite(relayPin, LOW);
    }
  }
}

float getTemp(){
 //returns the temperature from one DS18S20 in DEG Celsius

 byte data[12];
 byte addr[8];

 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
   Serial.println("CRC is not valid!");
   return -1000;
 }

 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   Serial.print("Device is not recognized");
   return -1000;
 }

 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end

 byte present = ds.reset();
 ds.select(addr);  
 ds.write(0xBE); // Read Scratchpad

 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];

 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;
 
 return TemperatureSum;
 
}
