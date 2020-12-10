
/* GPS Anzeige auf SSD1306 128x64 Display
GPS Neo-6 Modul wird benötigt
Erreichte Höchstgeschwindigkeit wird oben links angezeigt
Verbundene Satelliten oben rechts
WICHTIG: Zum flashen muß der 5V Anschluss vom GPS Modul getrennt werden */
// SDA = A4  SCL  = A5

#include <U8g2lib.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>                        //GPS Library

int gndPin = 2;                               // Pin 2 als Masse
int vccPin = 3;                               // Pin 3 auf 5V

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); 

#define u8g_logo_sat_width 20
#define u8g_logo_sat_height 20
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
static const int RXPin = 0, TXPin = 1;                    //RX auf TX, TX auf RX
// static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);

//GPS initialisieren
TinyGPSPlus gps;
int num_sat, gps_speed;

void setup() {
  delay(1000);
  u8g2.begin();
  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);    // PIN auf 5V setzen
  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);     // PIN auf Masse setzen
  ss.begin(9600); 
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
//Text km/h  
  u8g2.drawStr(72, 26, "km/h");
//Sat Logo - oben rechts
  u8g2.drawXBM(108, 0, u8g_logo_sat_width, u8g_logo_sat_height, u8g_logo_sat);
}

void loop() {
  Get_GPS();                              //Get GPS data      
  u8g2.firstPage();
  do {
    print_speed();
  } while ( u8g2.nextPage() );
} 
