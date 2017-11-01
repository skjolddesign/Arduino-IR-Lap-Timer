
/*
* IR Lap timer features:
 * by Per Emil Skjold 2012
 * 2 racers
   * one line for each racer
 * 16x2lcd
 * lap counter, lap time, best time in race
   * lap = 1x short beep
   * best race lap = 2x short beep
 * eeprom storage of best times(Delete Records with analog read button value. see sub deleteRecords)
   * pers.record car 1 = 4x fast beep
   * pers.record car 2 = 4x fast beep
 * ir timeout to prevent false trigg. see irTimeout 

TO DO:
* REACTION TIME IS TO SLOW. THIS MAY BE A TRANSMITTER PROBLEM


This project is built on:
* StopWatch
 * Paul Badger 2008
 * Demonstrates using millis(), pullup resistors, 
 * making two things happen at once, printing fractions
 */
 
#include <EEPROM.h>
#include <IRremote.h> // use the library for IR

int receiver = 2; // pin 1 of IR receiver to Arduino digital pin 2
IRrecv irrecv(receiver); // create instance of 'irrecv'
decode_results results;

#include <LiquidCrystal.h> 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define relePin  3
#define ledPin  13                  // LED connected to digital pin 13
#define buttonPin 12                 // button on pin 4


int irTimeout = 2; //IN SECONDS. TO PREVENT UNWANTED RETRIG. 10SEC irTimeout FOR A 14SEC LAP IS GOOD.
unsigned long currentMillis = 0; //verdi for total cpu tid til nå
long BestTimeInRace=40000;

//BIL 1
long startTime1 ;                    // start time for stop watch
long elapsedTime1 ;                  // elapsed time for stop watch
long lastTime1 = 0;                     // forrige tid
long endring1 = 0;                    //delta
long bestTime1 =30000;
String lapTimeText1 = "00";
String bestLapText1 = "Best 33 14.225s";
long previousMillisTrigg1 = 0;            // variable to store last time LED was updated
int fractional;                     // variable used to store fractional part of time
int runde1 = 1;
int started1 = 0;
int carFree1 = 1; //frigjort fra sperr
long recordCar1=30000;//rekord for lagring til eprom

//BIL 2
long startTime2 ;                    // start time for stop watch
long elapsedTime2 ;                  // elapsed time for stop watch
long lastTime2 = 0;                     // forrige tid
long endring2 = 0;                    //delta
long bestTime2 =30000;
String lapTimeText2 = "00";
String bestLapText2 = "Best 33 14.225s";
long previousMillisTrigg2 = 0;            // variable to store last time LED was updated
int fractiona2;                     // variable used to store fractional part of time
int runde2 = 1; //lap no
int started2 = 0; 
int carFree2 = 1; //frigjort fra sperr
long recordCar2=30000;//rekord for lagring til eprom

//relay timers
int rele1x=10;//disse settes til 0 for å starte rele
int lyd2=10;
int rele3x=10;
int rele6x=10;
int releState = HIGH;             // ledState used to set the LED
long previousMillisRele = 0;

//KEY PAD
char msgs[5][17] = {"Right Key      ",
                    "Up key         ",              
                    "Down key       ",
                    "Left Key       ",
                    "Select Key     "};
int adc_key_val[5] ={50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;

void setup()
{
   //IR
   Serial.begin(9600);
   irrecv.enableIRIn(); // Start the receiver
   irrecv.blink13(true); 
   //pins
   pinMode(ledPin, OUTPUT);         // sets the digital pin as output
   pinMode(relePin, OUTPUT);         // sets the digital pin as output
   digitalWrite(relePin, HIGH); 
   pinMode(buttonPin, INPUT);       // not really necessary, pins default to INPUT anyway
   digitalWrite(buttonPin, HIGH);   // turn on pullup resistors. Wire button so that press shorts pin to ground.
   Serial.println("Stoppeklokke klar");   
   //LCD
   lcd.begin(16, 2);
   lcd.clear();
   lcd.print("IR Lap Timer by");
   lcd.setCursor(0, 1); //linje2, (index0)
   lcd.print("Per Emil Skjold");
   delay(2000);
 
 //IR TIMOUT INFO
  lcd.clear();
  lcd.print("IR timeout = ");
  lcd.print(irTimeout);
  lcd.print("s");
  lcd.setCursor(0, 1); //linje2, (index0)
  lcd.print("Select=Slett tid");
  delay(3000);
 //SOUNDS
  lcd.clear();
  lcd.print("Varsel lyder:");
  lcd.setCursor(0, 1);
  lcd.print("1Kort = runde");
  delay(3000);
  lcd.clear();
  lcd.print("2korte= Bestetid");//lederbil i fremtiden??
  lcd.setCursor(0, 1);
  lcd.print("4Korte= Rekord");
  delay(3000);
 
 
 
 //RECORDS
  readRecord1(); //les eprom rekord
  readRecord2(); //les eprom rekord
  lcd.clear();
  lcd.print("Rekord 1 ");
  lcd.print(print_time(recordCar1));
  lcd.setCursor(0, 1); //linje2, (index0)
  lcd.print("Rekord 2 ");
  lcd.print(print_time(recordCar2));
  



}

void loop()
{
  currentMillis = millis();
  
 rele1sub(); //sjekk om rele skal aktiveres
 rele1Langsub();
 rele3sub(); //sjekk om rele skal aktiveres
  sperrstatus1(); //sjekk sperrstatus
 sperrstatus2();
  
  //debug
  //delay(4000);
  // trigg();
  //IR
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR();
    //tatt bort resume loop, for jeg vil ha alle koder
//    for (int z=0; z<2; z++) // ignore 2nd and 3rd signal repeat
//    {
      irrecv.resume(); // receive the next value
//    }
  }
readAnalog();
}

void trigg1()
{   //SPERR
    //kjør kun vist total cpu tid - forrige cpu tid for bil1 er større enn valgt timeout
     if(carFree1==1) { //sjekk lås
     carFree1=0;//lås
     
     lcd.setCursor(15, 0); 
     lcd.print("*"); //symboliser
     elapsedTime1 =   currentMillis - startTime1;              // store elapsed time
    //reset tid-----------------------------
     startTime1 = currentMillis;                                   // store the start time

     //kjør en gang-------------------------
      if (started1 == 0){
      started1=1; //timer is started 
      rele1x = 0;
     Serial.println("Car 1 started");   
      lcd.setCursor(0, 0);
      lcd.print("Car 1 started  ");
      }
      else
      {
       
       {    
      
         // routine to report elapsed time 
       //serial
       Serial.print("Lap "); 
       Serial.print(runde1); 
       Serial.print("   "); 
       lapTimeText1=print_time(elapsedTime1);
       Serial.print(lapTimeText1); //ms long til sekund
       Serial.print("    "); 
      // endring1=elapsedTime1 -  lastTime1;
      // Serial.println(endring1); 
      Serial.println(elapsedTime1); 
  
   //runde tekst
   lcd.setCursor(0, 0); 
// lcd.print("Lap      ");
// lcd.setCursor(5, 0); 
   lcd.print(runde1);
   lcd.print("  "); //SKRIV VIDERE
  
     //tid forbedring
//   lcd.setCursor(0, 1); 
//   lcd.print(String(endring) + "ms");
   
    //tid runde
    lcd.setCursor(3, 0); 
    lcd.print(lapTimeText1); //ms long til sekund
    lcd.setCursor(8, 0); 
    lcd.print("  "); //visker over siste siffer

//      //LØPS REKORD, LA DEN BESTE BIL SYNLIG GJØRES
//      if ((elapsedTime1 <= bestTime2)&&(elapsedTime1 <= bestTime1)){ //vist bedre enn andre bil og egen bestetid
//      rele3antall=0;
//      }
    
    //LØPS BASERT, IKKE ALLTIME
    //LEDERBIL SYNLIGGJØRES
    //RUNDE = 1PIP
    //NY BESTETID = 2PIP (for å vise forbedringer)
    //NY REKORD = 4PIP
    
    
    //EGEN REKORD SLÅTT. LAGRE. IKKE VIS DENNE BEGIVENHETEN
       if (elapsedTime1 <= recordCar1){  //vist bedre enn bil 2 også
       recordCar1=elapsedTime1;  //PERS VERDI
       writeRecord1();    
       }
     //BESTETID PERS LØP 1
     if (elapsedTime1 < bestTime1){
      bestTime1=elapsedTime1;   //set ny bestetid
      lcd.setCursor(10, 0);
      lcd.print(lapTimeText1);
      lcd.setCursor(15, 0); 
      lcd.print("*"); //SETT TILBAKE STJERNE DA DEN ER SKREVET OVER OVENFOR
     
        //REKORDER SLÅTT =4PIP
        if ((elapsedTime1 <= recordCar1)&&(elapsedTime1 <= recordCar2))
        rele3x = 0; //NY REKORD
        //LØPS BESTE (LEDER BIL) = 2PIP
             
        //PERS BESTETID SLÅTT = 2PIP (for å vise forbedringer)
        else
        lyd2 = 0; //KJØRER VANLIG
     }
     
       //VANLIG RUNDE 1PIP
     else{
     rele1x = 0; //KJØRER VANLIG
     }
  
  
   //neste runde------------------------------
   runde1++;
   lastTime1=elapsedTime1; //husk tid    
   }
      }
      
    //----------------------------------------  
    //peepRunde
   
     }
}




//BIL 2
void trigg2()
{   //SPERR
    //kjør kun vist total cpu tid - forrige cpu tid for bil1 er større enn valgt timeout
     if(carFree2==1) { //sjekk lås
     carFree2=0;//lås
     
     lcd.setCursor(15, 1); 
     lcd.print("*"); //symboliser
     elapsedTime2 =   currentMillis - startTime2;              // store elapsed time
    //reset tid-----------------------------
     startTime2 = currentMillis;                                   // store the start time

     //kjør en gang-------------------------
      if (started2 == 0){
      started2=1; //timer is started 
      rele1x = 0;
      Serial.println("Car 2 started");   
      lcd.setCursor(0, 1);
      lcd.print("Car 2 started  ");
      }
      else
      {
         {
           
      
         // routine to report elapsed time 
       //serial
       Serial.print("Car 2 Lap "); 
       Serial.print(runde2); 
       Serial.print("  "); 
       lapTimeText2=print_time(elapsedTime2);
       Serial.print(lapTimeText2); //ms long til sekund
       Serial.print("   "); 
      // endring1=elapsedTime1 -  lastTime1;
      // Serial.println(endring1); 
      Serial.println(elapsedTime2); 
  
   //runde tekst
   lcd.setCursor(0, 1); 
// lcd.print("Lap      ");
// lcd.setCursor(5, 0); 
   lcd.print(runde2);
   lcd.print("  ");
  
     //tid forbedring
//   lcd.setCursor(0, 1); 
//   lcd.print(String(endring) + "ms");
   
    //tid runde
    lcd.setCursor(3, 1); 
    lcd.print(lapTimeText2); //ms long til sekund
    lcd.setCursor(8, 1); 
    lcd.print("  "); //visker over siste siffer
    



      //PERS REKORD 2
      //EGEN REKORD SLÅTT. LAGRE. IKKE VIS DENNE BEGIVENHETEN
       if (elapsedTime2 <= recordCar2){  //vist bedre enn bil 2 også
       recordCar2=elapsedTime2;  //PERS VERDI
       writeRecord2();    
       }
     //BESTETID PERS LØP 2
     if (elapsedTime2 < bestTime2){
      bestTime2=elapsedTime2;   //set ny bestetid
      lcd.setCursor(10, 1);
      lcd.print(lapTimeText2);
      lcd.setCursor(15, 1); 
      lcd.print("*"); //SETT TILBAKE STJERNE DA DEN ER SKREVET OVER OVENFOR
     
        //REKORDER SLÅTT =4PIP
        if ((elapsedTime2 <= recordCar2)&&(elapsedTime2 <= recordCar1))
        rele3x = 0; //NY REKORD
        //LØPS BESTE (LEDER BIL) = 2PIP
             
        //PERS BESTETID SLÅTT = 2PIP (for å vise forbedringer)
        else
        lyd2 = 0; //KJØRER VANLIG
     }
     
       //VANLIG RUNDE 1PIP
     else{
     rele1x = 0; //KJØRER VANLIG
     }
  
   //neste runde------------------------------
   runde2++;
   lastTime2=elapsedTime2; //husk tid    
   }
      }
 

}
}



String longToSec(unsigned long t_milli) //var Void før
{
    String text;
    
 
}
//Here's a print routine that can convert a long int, corresponding to milliseconds, 
//to days, hours, minutes, seconds and milliseconds and print to the LCD.
// argument is time in milliseconds
String print_time(unsigned long t_milli) //var Void før
{
    char buffer[20];
    int days, hours, mins, secs;
    int fractime;
    unsigned long inttime;
    String text;
    
    inttime  = t_milli / 1000;
    fractime = t_milli % 1000; //100=2decimaler
    
   
    // inttime is the total number of number of seconds
    // fractimeis the number of thousandths of a second

    // number of days is total number of seconds divided by 24 divided by 3600
    days     = inttime / (24*3600);
    inttime  = inttime % (24*3600);

    // Now, inttime is the remainder after subtracting the number of seconds
    // in the number of days
    hours    = inttime / 3600;
    inttime  = inttime % 3600;

    // Now, inttime is the remainder after subtracting the number of seconds
    // in the number of days and hours
    mins     = inttime / 60;
    inttime  = inttime % 60;

    // Now inttime is the number of seconds left after subtracting the number
    // in the number of days, hours and minutes. In other words, it is the
    // number of seconds.
    secs = inttime;

    // Don't bother to print days
//     sprintf(buffer, "%02d:%02d:%02d.%03d", hours, mins, secs, fractime);
       sprintf(buffer, "%02d.%02d", secs, fractime);
  
//     lcd.print(buffer);
// text=buffer;
//  text=text.substring(0,5); //kort ned lengde slik at en bare har 2 desimaler
//  text=text-1;
       return buffer;
 
}


void translateIR()
// takes action based on IR code received
// describing Sony IR codes on LCD module
{
    Serial.print("Data=");
   Serial.println(results.value, HEX);
  switch(results.value)
  {
   //Ariels DVD REMOTE, FOR DEBUGING
    case 0xF60: Serial.println("RC5 ned");  trigg1(); break;
    case 0x760: Serial.println("RC5 ned");  trigg1(); break;
    case 0xF61:  Serial.println("RC5 opp");  trigg2(); break;
    case 0x761:  Serial.println("RC5 opp");  trigg2(); break;
    
    //4BIT RC6 CODE FROM CAR1. 4 is fastest.
    case 0x1:  Serial.println("Car 1 passed");  trigg1(); break;
    //4BIT RC6 CODE FROM CAR2. 4 is fastest.
    case 0x2:  Serial.println("Car 2 passed");  trigg2(); break; 

//    case 0xA90:  Serial.println("Power button"); break;
//    case 0x290:  Serial.println("mute           "); break;
    
    //SONY KODER
//    case 0x10:   trigg1();Serial.println("Sony one");  break;
//    case 0x810:  trigg2();Serial.println("Sony two"); break;

//    case 0x410:  Serial.println("three          "); break;
//    case 0xC10:  Serial.println("four           "); break;
//    case 0x210:  Serial.println("five           "); break;
//    case 0xA10:  Serial.println("six            "); break;
//    case 0x610:  Serial.println("seven          "); break;
//    case 0xE10:  Serial.println("eight          "); break;
//    case 0x110:  Serial.println("nine           "); break;
//    case 0x910:  Serial.println("zero           "); break;
//    case 0x490:  Serial.println("volume up      "); break;
//    case 0xC90:  Serial.println("volume down    "); break;
//    case 0x90:   Serial.println("channel up     "); break;
//    case 0x890:  Serial.println("channel down   "); break;
//    default: Serial.println(" other button   ");
  }
  //delay(2000); 
  
}

//runde=1x rask
void rele1sub()
{
 if (rele1x == 0){ //vist 0, altså resatt skal denne kjøre. startes ved å sette denne til 0
 
  if(currentMillis - previousMillisRele > 100) {
    
    // save the last time you blinked the LED 
    previousMillisRele = currentMillis;   
 // if the LED is off turn it on and vice-versa:
    if (releState == LOW) //rele på
      releState = HIGH, rele1x=1;
    else
      releState = LOW; //rele av
 // set the LED with the ledState of the variable:
    digitalWrite(relePin, releState);
  }
} 
}

//person rekord =1xlang
void rele1Langsub()
{
 if (lyd2 <= 1){
  if(currentMillis - previousMillisRele > 70) {
    // save the last time you blinked the LED 
    previousMillisRele = currentMillis;   
 // if the LED is off turn it on and vice-versa:
    if (releState == LOW)
      releState = HIGH, lyd2++; //rele av
    else
      releState = LOW; //rele på
 // set the LED with the ledState of the variable:
    digitalWrite(relePin, releState);
  }
} 
}


//lederbil spiller 3x raske
void rele3sub()
{
 if (rele3x <= 4){
  if(currentMillis - previousMillisRele > 70) {
    // save the last time you blinked the LED 
    previousMillisRele = currentMillis;   
 // if the LED is off turn it on and vice-versa:
    if (releState == LOW)
      releState = HIGH, rele3x++; //rele av
    else
      releState = LOW; //rele på
 // set the LED with the ledState of the variable:
    digitalWrite(relePin, releState);
  }
} 
}

void sperrstatus1()
{
  //frigjør sperr for bil 1
   if (started1==1)//TRENGER IKKE KLAMMER UNDER EN IF SETNING :)
   if(currentMillis - startTime1 > (irTimeout * 1000)) { //sjekk forige gang trigg skjedde
     carFree1 = 1;
     lcd.setCursor(15, 0);
    lcd.print(" "); //symboliser
       }
     //else
    // carFree1 = 0; 
     
}


void sperrstatus2()
{
  //frigjør sperr for bil 1
   if (started2==1)//TRENGER IKKE KLAMMER UNDER EN IF SETNING :)
   if(currentMillis - startTime2 > (irTimeout * 1000)) { //sjekk forige gang trigg skjedde
     carFree2 = 1;
     lcd.setCursor(15, 1);
     lcd.print(" "); //symboliser
       }
     //else
    // carFree1 = 0; 
 }
 
void readRecord1(){
  
byte hiByte = EEPROM.read(0);
byte loByte = EEPROM.read(1);
 recordCar1 = (hiByte << 8)+loByte;
}

//lagrer 2 byte(integer)= maks 32000
void writeRecord1(){
byte loByte = byte(recordCar1);
byte hiByte = byte(recordCar1 >> 8);
 EEPROM.write(0, hiByte);
 EEPROM.write(1, loByte);
}
void readRecord2(){
  
byte hiByte = EEPROM.read(2);
byte loByte = EEPROM.read(3);
 recordCar2 = (hiByte << 8)+loByte;
}

void writeRecord2(){
byte loByte = byte(recordCar2);
byte hiByte = byte(recordCar2 >> 8);
 EEPROM.write(2, hiByte);
 EEPROM.write(3, loByte);
}
void deleteRecords(){
 recordCar1=32000; //maks lager størrelse 2byte (int)=32000
 recordCar2=32000;
 writeRecord1();
 writeRecord2();
 lcd.clear();
 lcd.print("Rekorder slettet");
}

void readAnalog(){
   adc_key_in = analogRead(0);    // read the value from the sensor
  key = get_key(adc_key_in);  // convert into key press
 
  if (key != oldkey)   // if keypress is detected
   {
      // ONE SHOT EVENTS ON KLICK DOWN
      doKeyDown();
    
    delay(50);  // wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor
    key = get_key(adc_key_in);    // convert into key press
    if (key != oldkey)   
    {  
      lcd.setCursor(0, 1);
      oldkey = key;
      if (key >=0){
           lcd.print(msgs[key]);             
      }
    }
  }
 }
 //*************************************************************************** 

void doKeyDown()
{
    switch(key)
  {
 // case 0: rele1x=0; break; //right
 // case 1: rele1x=0; break; //up
//  case 2: rele1x=0; break; //down
 // case 3: rele3x=0; break; //left
    case 4: deleteRecords(); break; //select key

//    default: Serial.println(" other button   ");
  }
  
//   if (key == 4){ //DOWN
//       }
//       
//   if (key == 1){ //SELECT
//       rele1x=0;
//      }  
//         
//   if (key == 2){ //LEFT
//         } 
// delay (50);  //for å unngå ripple input i brytings tidspunktet.
}

//***************************************************************************
// Convert ADC value to key number
int get_key(unsigned int input)
{
    int k;
    for (k = 0; k < NUM_KEYS; k++)
    {
      if (input < adc_key_val[k])
 {
            return k;
        }
   }
    if (k >= NUM_KEYS)k = -1;  // No valid key pressed
    return k;
}      

