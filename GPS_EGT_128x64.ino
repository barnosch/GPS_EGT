/* GPS und EGT Anzeige auf SSD1306 128x64 Display
GPS Modul und Max6675 Temparatur Fühler wird benötigt
Erreichte Höchstgeschwindigkeit wird oben links angezeigt
Verbundene Satelitten oben rechts
LED auf PIN 13 lässt sich für bestimmte Schwellenwerte definieren

WICHTIG: Zum flashen muß der 5V Anschluss vom GPS Modul getrennt werden */
#include <U8g2lib.h>
#include "max6675.h"
#include <SoftwareSerial.h>
//GPS Library
#include <TinyGPS++.h>

  int gndPin = 2;             // Pin 2 als Masse
  int vccPin = 3;             // Pin 3 auf 5V
  int ktcSO = 6;           
  int ktcCS = 5;
  int ktcCLK = 4;          
  MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); 

#define u8g_logo_sat_width 20
#define u8g_logo_sat_height 20
const int DEL = 150;          //delay. ohne das keine Tempaktualisierung
int val = 0;
char out[6];
String str;
int LED_blink = 570;         // Schwellenwert ab wann die LED blinkt
int LED_full  = 600;         // Schwellenwert ab wann die LED dauerhaft leuchtet
int maxkmh = 0;

//satellite logo
const unsigned char u8g_logo_sat[] = {
  0x00, 0x01, 0x00, 0x80, 0x07, 0x00, 0xc0, 0x06, 0x00, 0x60, 0x30, 0x00,
  0x60, 0x78, 0x00, 0xc0, 0xfc, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xff, 0x01,
  0x80, 0xff, 0x00, 0xc0, 0x7f, 0x06, 0xc0, 0x3f, 0x06, 0x80, 0x1f, 0x0c,
  0x80, 0x4f, 0x06, 0x19, 0xc6, 0x03, 0x1b, 0x80, 0x01, 0x73, 0x00, 0x00,
  0x66, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x70, 0x00, 0x00
};

// The serial connection to the GPS device
static const int RXPin = 0, TXPin = 1; //RX auf TX, TX auf RX
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);

//GPS initialisieren
TinyGPSPlus gps;
int num_sat, gps_speed;

void setup() {
  delay(1000);
  u8g2.begin();
  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);    // PIN auf 5V setzen
  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);     // PIN auf Masse setzen
  ss.begin(GPSBaud); 
}

void Get_GPS()
{
  num_sat = gps.satellites.value();
  if (gps.location.isValid() == 1) {
    gps_speed = gps.speed.kmph();
    if(gps_speed > maxkmh)
    {
      maxkmh = gps_speed;
    }
  }
smartDelay(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
  // Serial.println(F("No GPS detected: check wiring."));
  }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void print_speed() {
  //rechts unten
  u8g2.setFont(u8g2_font_fub30_tr);
  if (gps_speed < 100) { 
   u8g2.setCursor(80,60);
   u8g2.print(gps_speed , DEC);
   } else{
      u8g2.setCursor(61,60); 
      u8g2.print(gps_speed , DEC);
  }
  
//Anzahl Topspeed, oben links
u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(1, 12, "Speed ");
  u8g2.setCursor(34,13); 
  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.print(maxkmh); 
//Anzahl der Satelliten, oben rechts
  u8g2.setCursor(88, 13);
  u8g2.print( num_sat, 5);
// Text EGT
  u8g2.drawStr(12, 26, "EGT");
//Text km/h  
  u8g2.drawStr(72, 26, "km/h");
//Sat Logo - oben rechts
  u8g2.drawXBM(108, 0, u8g_logo_sat_width, u8g_logo_sat_height, u8g_logo_sat);
}

void egtemp(){ //links unten
  if (val < 100) { 
    u8g2.setFont(u8g2_font_helvB24_tr);
    u8g2.drawStr(0,58, out);
    u8g2.drawCircle(40,35,3,U8G2_DRAW_ALL); 
    } else{
      u8g2.setFont(u8g2_font_helvB24_tr); 
      u8g2.drawStr(0, 58, out);
      u8g2.drawCircle(55,35,3,U8G2_DRAW_ALL); 
  }
}

void loop() {
  Get_GPS();                     //Get GPS data
  val = ktc.readCelsius();
  str = String(val);
  str.toCharArray(out,6);        //EGT Anzeige
  delay(DEL);      
  //Display info in the OLED
  u8g2.firstPage();
  do {
    print_speed();
    egtemp();
  } while ( u8g2.nextPage() );

// LED blinken bei Temperatur groesser als Wert LED_blink (oben definiert)
if (val > LED_blink )
 { 
  digitalWrite(13, HIGH);       // Pin 13 soll auf HIGH ( 5 Volt ) gesetzt werden
  delay(500);                   // Warte 500 Millisekunden (1000 = 1 Sekunde)
  digitalWrite(13, LOW);        // Pin 13 soll auf LOW ( 0 Volt/Masse ) gesetzt werden                  
  }
// dauerhaft leuchten, wenn über Schwellenwert "LED_full" (oben definiert)
if (val >= LED_full )
  { 
    digitalWrite(13, HIGH);       // Pin 13 soll auf HIGH ( 5 Volt ) gesetzt werden
  }
} 
