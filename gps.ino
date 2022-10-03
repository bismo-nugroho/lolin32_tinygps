#include <arduino.h>
#include <TinyGPSPlus.h>
//#include <NeoSWSerial.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>          // include Arduino time library
/*
   This sample code demonstrates just about every built-in operation of TinyGPSPlus (TinyGPSPlus).
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 32, TXPin = 33;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
//    NeoSWSerial ss(RXPin, TXPin);


// GPS Port
#define RX_GPS 32
#define TX_GPS 33

//Serial GPS
//SoftwareSerial gps_port;//(RX_GPS, TX_GPS);




float getLat(void);
float getLon(void);
String getDateTimeLoc(void);



byte flipsetup = 0;
byte flipchange = 0;
byte flipsetupset = 0;
bool valid_location = false;
bool valid_time = false;
bool buzzer = false, buzzeron = false, buzzers = true;
bool soundon = false;

double timertone = 0;
bool soundtone = false, sound = false;



byte MIN_KPH = 3;
double UPDATE_INTERVAL = 3;// every 5 seconds get save location
int counter = 0;
int lastcounter = -1;
int maxcounter = 4;
byte currentStateCLK;
byte lastStateCLK;
bool currentDir = 0;
bool invert = false, invertset = false;
bool usbdebug = true, usbdebugset = false;
int adjhour = 7;
int adjhourset = 7;
int adjspeed, adjspeedset = 0;
byte adjwarn, adjwarnset = 0;
byte setting, saving = 0;
String heading = "";
float heading_cd;
bool confsave = false; //
double lastButtonPress = 0;
//int lastButtonPress = 0;
byte nmea = 0;
byte lnmea = 0;
bool blink = false;
char imei[15];
char unique[16];

int initimei = 0;
int countdown = 0;
int idxcount = 0;
int settingload = 1;

String datelocal = "";

char timeloc[9];
char dateloc[11];


int delaypush = 300;
int delayhold = 1500;

void getSaved(void);
void setTZ(void);


double timestartgps = 0;
double timefixgps = 0;

int contentflag = 0;
CircularBuffer<String, 15> contentsms;
CircularBuffer<String, 30> serialdata;


// variable definitions
char Time[]  = "00:00:00";
char Date[]  = "00-00-2000";
byte last_second, Second, Minute, Hour, Day, Month;
int Year;

/*
  int signalstat = 0;
  int netstat = 0;
  String ipaddr = "";
  int inetstat = 0;
  bool result = 0;
  int atstat = 0;
  int initstat = -1;

  String atmsg = "";
  String flagat = "";
*/
byte speeds = 0;

//satellite logo
const unsigned char u8g_logo_sat[] = {
  0x04, 0x00, 0x0A, 0x00, 0x11, 0x00, 0x22, 0x00, 0xE4, 0x00, 0xF8, 0x00,
  0xF0, 0x01, 0x74, 0x02, 0x44, 0x04, 0x9D, 0x08, 0x01, 0x05, 0x07, 0x02
};



const unsigned char  logo_envelope[] = {
  0xff, 0xff, 0x0f, 0xff, 0xff, 0x0f, 0xff, 0xff, 0x0f, 0x03, 0x00, 0x0e, 0xe3, 0x3f, 0x0e,
  0xcb, 0x9f, 0x0e, 0xdb, 0xcf, 0x0e, 0x3b, 0xe7, 0x0e, 0x3b, 0xe0, 0x0e, 0x9b, 0xc8, 0x0e,
  0xcb, 0x9d, 0x0e, 0xe3, 0x3f, 0x0e, 0x03, 0x00, 0x0e, 0xff, 0xff, 0x0f, 0xff, 0xff, 0x0f,
  0xff, 0xff, 0x0f, 0xff, 0xff, 0x0f
};



const unsigned char  logo_start[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xf0, 0xff, 0x00, 0x06, 0x00, 0x80, 0xff, 0x0f, 0x30, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01,
  0x00, 0xfe, 0xff, 0xcf, 0x07, 0x00, 0xf8, 0xff, 0xff, 0x3c, 0x00, 0xfc, 0xff, 0xff, 0xff,
  0x0f, 0x80, 0xff, 0xff, 0xff, 0x07, 0x00, 0xff, 0xff, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0xff,
  0xff, 0x3f, 0xc0, 0x3f, 0xc0, 0xff, 0x07, 0xc0, 0xff, 0xff, 0xff, 0x3f, 0x00, 0xe0, 0xff,
  0xff, 0xff, 0x7f, 0xc0, 0x1f, 0x00, 0xfc, 0x07, 0xe0, 0xff, 0x07, 0xfc, 0x3f, 0x00, 0xc0,
  0xff, 0x03, 0xf0, 0xff, 0xe0, 0x3f, 0x00, 0xf0, 0x07, 0xf0, 0xff, 0x01, 0xc0, 0x3f, 0x00,
  0x80, 0xff, 0x03, 0xe0, 0xff, 0xe1, 0xff, 0x0f, 0xc0, 0x07, 0xf8, 0x7f, 0x00, 0x00, 0x3e,
  0x00, 0x80, 0xff, 0x03, 0xe0, 0xff, 0xe1, 0xff, 0xff, 0x1f, 0x07, 0xfc, 0x7f, 0x00, 0x00,
  0x38, 0x00, 0x80, 0xff, 0x03, 0xe0, 0xff, 0xe1, 0xff, 0xff, 0xff, 0x07, 0xfe, 0x3f, 0x00,
  0x00, 0x30, 0x00, 0x80, 0xff, 0x03, 0xf0, 0xff, 0xc1, 0xff, 0xff, 0xff, 0x0f, 0xfe, 0x3f,
  0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0x1f, 0xfe,
  0x3f, 0xf8, 0xff, 0xff, 0x1f, 0x80, 0xff, 0xff, 0xff, 0x7f, 0x80, 0xff, 0xff, 0xff, 0x3f,
  0xfe, 0x3f, 0xe0, 0xff, 0xff, 0x07, 0x80, 0xff, 0xff, 0xff, 0x1f, 0xc0, 0xfe, 0xff, 0xff,
  0x7f, 0xfe, 0x3f, 0x80, 0xff, 0xff, 0x01, 0x80, 0xff, 0xff, 0xff, 0x03, 0xc0, 0xf1, 0xff,
  0xff, 0x7f, 0xfc, 0x7f, 0x00, 0xfe, 0xff, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00, 0xc0, 0x03,
  0xf8, 0xff, 0x7f, 0xfc, 0x7f, 0x00, 0xf8, 0x3f, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00, 0xc0,
  0x0f, 0x00, 0xe0, 0x7f, 0xf8, 0xff, 0x00, 0xf8, 0x3f, 0x00, 0xc0, 0xff, 0x07, 0x00, 0x00,
  0xc0, 0x7f, 0x00, 0xc0, 0x7f, 0xf0, 0xff, 0x03, 0xfe, 0x3f, 0x00, 0xe0, 0xff, 0x0f, 0x00,
  0x00, 0xc0, 0xff, 0x07, 0xe0, 0x3f, 0xe0, 0xff, 0xff, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0x3f,
  0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x1f, 0x80, 0xff, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0xff,
  0x7f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xfc, 0xff, 0xff, 0x3c, 0x00, 0xff,
  0xff, 0xff, 0x01, 0x00, 0xc0, 0xe3, 0xff, 0xff, 0x03, 0x00, 0xc0, 0xff, 0x07, 0x30, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
  0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x3f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x3f, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x3f, 0xfe, 0x77, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xef,
  0xff, 0xff, 0xff, 0x3f, 0xfe, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff,
  0xef, 0xff, 0xff, 0xff, 0x3f, 0xfe, 0xfb, 0x2f, 0xfc, 0xf0, 0xc3, 0x0f, 0x3d, 0x3c, 0xf0,
  0xc3, 0x03, 0x3f, 0xfc, 0xe2, 0x3f, 0xfe, 0xfb, 0xcf, 0x7b, 0xef, 0xbd, 0xf7, 0xdc, 0xbb,
  0xed, 0xbd, 0xef, 0xdf, 0xfb, 0xdc, 0x3f, 0xfe, 0xc7, 0xef, 0xb7, 0xdf, 0x7e, 0xfb, 0xed,
  0xb7, 0xed, 0x7e, 0xef, 0xef, 0xf7, 0xfe, 0x3f, 0xfe, 0x1f, 0xee, 0xb7, 0xdf, 0x7e, 0xfb,
  0xed, 0xb7, 0xed, 0x7e, 0xef, 0xef, 0xf7, 0xfe, 0x3f, 0xfe, 0xff, 0xed, 0x37, 0xc0, 0x00,
  0xfb, 0xed, 0xb7, 0xed, 0x00, 0xef, 0x0f, 0xf0, 0xfe, 0x3f, 0xfe, 0xff, 0xed, 0xb7, 0xff,
  0xfe, 0xfb, 0xed, 0xb7, 0xed, 0xfe, 0xef, 0xef, 0xff, 0xfe, 0x3f, 0xfe, 0xfb, 0xed, 0xb7,
  0xff, 0xfe, 0xfb, 0xed, 0xb7, 0xed, 0xfe, 0xef, 0xef, 0xff, 0xfe, 0x3f, 0xfe, 0xf3, 0xcc,
  0x7b, 0xdf, 0x7d, 0xf7, 0xdc, 0xbb, 0xed, 0x7d, 0xef, 0xdf, 0xf7, 0xfe, 0x3f, 0xfe, 0x07,
  0x2f, 0xfc, 0xe0, 0x83, 0x0f, 0x3d, 0xbc, 0xed, 0x83, 0x1f, 0x3f, 0xf8, 0xfe, 0x3f, 0xfe,
  0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
  0xfe, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x3f, 0xfe, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x3f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x0c, 0x8c, 0x01, 0x80,
  0x0f, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x0c, 0x8c, 0x01,
  0x80, 0x19, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x0c, 0x8c,
  0x01, 0x80, 0x31, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x8c, 0x87, 0x87, 0x0d,
  0x8c, 0xd9, 0x80, 0x31, 0xf3, 0x6c, 0x8c, 0x67, 0x0c, 0x00, 0x00, 0xe0, 0xcf, 0x4c, 0xcc,
  0x0e, 0x8c, 0xb9, 0x81, 0x31, 0x9b, 0xdd, 0x4c, 0x6c, 0x0c, 0x00, 0x00, 0x60, 0xcc, 0x0c,
  0xcf, 0x0c, 0x8c, 0x99, 0x81, 0x31, 0x3b, 0xcc, 0x0c, 0xcf, 0x06, 0x00, 0x00, 0x60, 0xcc,
  0x8f, 0xcd, 0x0c, 0x8c, 0x99, 0x81, 0x31, 0xf3, 0xcc, 0x8c, 0xcd, 0x06, 0x00, 0x00, 0x60,
  0xcc, 0xc0, 0xcc, 0x0c, 0x8c, 0x99, 0x81, 0x31, 0xc3, 0xcd, 0xcc, 0xcc, 0x06, 0x00, 0x00,
  0x60, 0xcc, 0xcc, 0xcc, 0x0e, 0x8c, 0xb9, 0x81, 0x19, 0x9b, 0xdd, 0xcc, 0x8c, 0x03, 0x00,
  0x00, 0x60, 0x8c, 0x87, 0x8f, 0x0d, 0xf8, 0xd8, 0x80, 0x0f, 0xf3, 0x6c, 0x8c, 0x8f, 0x03,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x80,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x00,
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0c,
  0x00, 0xe0, 0x01, 0x00
};



//Program variables
double Lat, oLat = 0;
double Long, oLong = 0;

double lastlat;
double lastlong;

double odometer = 0;
double lastodometer = 0;

float gps_speed = 0;
float hdop = 0;
int alt;
byte num_sat, satinview, page, pages;

//String heading;
//static NMEAGPS gps; // This parses the GPS characters
double timer, timers, blank, msg, timerb, timerupdate;
//int timer, timers, blank, msg;
//NeoGPS::time_t localTime; //(utcTime + (UTC_offset * (60 * 60)));

//include "espnow";

#define U8LOG_WIDTH 20
#define U8LOG_HEIGHT 8
//uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];
//U8G2LOG u8g2log;


static const int MAX_SATELLITES = 40;

TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element
TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
TinyGPSCustom satNumber[4]; // to be initialized later
TinyGPSCustom elevation[4];
TinyGPSCustom azimuth[4];
TinyGPSCustom snr[4];

struct
{
  bool active;
  int elevation;
  int azimuth;
  int snr;
} sats[MAX_SATELLITES];





float getLat() {
  if (valid_location) {
    return  Lat;
  }
  return 0.00000;
}

float getLon() {
  if (valid_location) {
    return   Long;
  }
  return 0.00000;
}


String getDateTimeLoc() {
  return datelocal;
}

gpsdata_struct getGPSData() {
  gpsdata_struct data;
  data.Lat = Lat;
  data.Long = Long;
  data.odometer = odometer;
  data.alt = alt;
  if (gps_speed > MIN_KPH && gps_speed < 150 && Lat < 0 && Long > 80)// and hdop <= 3000 and hdop > 0 )
  {
    data.gps_speed = gps_speed;
  } else {
    data.gps_speed = 0;
  }
  data.num_sat = num_sat;
  data.satinview = satinview;
  data.datelocal = datelocal;
  data.valid_location = valid_location;

  return data;

}


char *cardinal(double course)
{
  char *directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
  int direction = (int)((course + 11.25f) / 22.5f);
  return directions[direction % 16];
}




void gps_setup() {
  //gps_port.begin(GPSBaud, SWSERIAL_8N1, RX_GPS, TX_GPS, false);

    ss.begin(GPSBaud);


  Serial.println(F("SatelliteTracker.ino"));
  Serial.println(F("Monitoring satellite location and signal strength using TinyGPSCustom"));
  Serial.print(F("Testing TinyGPSPlus library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
  
  // Initialize all the uninitialized TinyGPSCustom objects
  for (int i=0; i<4; ++i)
  {
    satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
    elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
    azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
    snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
  }
}


String getIDStart(NeoGPS::time_t localtime) {
  String res = "";
  res.concat(localtime.full_year());

  if (localtime.month < 10)
    res.concat("0");
  res.concat(localtime.month);

  if (localtime.date < 10)
    res.concat("0");

  if (localtime.hours < 10)
    res.concat("0");

  res.concat(localtime.hours);

  if (localtime.minutes < 10)
    res.concat("0");

  res.concat(localtime.minutes);


  if (localtime.seconds < 10)
    res.concat("0");

  res.concat(localtime.seconds);
  return res;
}

void print_wday(byte wday)
{
  /*
  lcd.setCursor(5, 1);  // move cursor to column 5, row 1
  switch(wday)
  {
    case 1:  lcd.print(" SUNDAY  ");   break;
    case 2:  lcd.print(" MONDAY  ");   break;
    case 3:  lcd.print(" TUESDAY ");   break;
    case 4:  lcd.print("WEDNESDAY");   break;
    case 5:  lcd.print("THURSDAY ");   break;
    case 6:  lcd.print(" FRIDAY  ");   break;
    default: lcd.print("SATURDAY ");
  }
  */

}


/* static */
double long distanceBetween(double lat1, double long1, double lat2, double long2)
{
  double delta = radians(long1 - long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}

String getdatetime(NeoGPS::time_t localtime) {
  String res = "";
  res.concat(localtime.full_year());
  res.concat("-");

  if (localtime.month < 10)
    res.concat("0");
  res.concat(localtime.month);

  res.concat("-");
  if (localtime.date < 10)
    res.concat("0");
  res.concat(localtime.date);

  res.concat(" ");

  if (localtime.hours < 10)
    res.concat("0");
  res.concat(localtime.hours);
  res.concat(":");
  if (localtime.minutes < 10)
    res.concat("0");

  res.concat(localtime.minutes);
  res.concat(":");
  if (localtime.seconds < 10)
    res.concat("0");

  res.concat(localtime.seconds);
  return res;
}


void gps_handle() {

 // Dispatch incoming characters
  while (ss.available() > 0)
    gps.encode(ss.read());

  if (gps.location.isUpdated())
  {
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);

        valid_location = true;
    Lat = gps.location.lat();
    Long = gps.location.lng();
  }

  else if (gps.date.isUpdated())
  {
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  }

  else if (gps.time.isUpdated())
  {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }

  else if (gps.speed.isUpdated())
  {
    Serial.print(F("SPEED      Fix Age="));
    Serial.print(gps.speed.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.speed.value());
    Serial.print(F(" Knots="));
    Serial.print(gps.speed.knots());
    Serial.print(F(" MPH="));
    Serial.print(gps.speed.mph());
    Serial.print(F(" m/s="));
    Serial.print(gps.speed.mps());
    Serial.print(F(" km/h="));
    Serial.println(gps.speed.kmph());
  }

  else if (gps.course.isUpdated())
  {
    Serial.print(F("COURSE     Fix Age="));
    Serial.print(gps.course.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.course.value());
    Serial.print(F(" Deg="));
    Serial.println(gps.course.deg());
  }

  else if (gps.altitude.isUpdated())
  {
    Serial.print(F("ALTITUDE   Fix Age="));
    Serial.print(gps.altitude.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.altitude.value());
    Serial.print(F(" Meters="));
    Serial.print(gps.altitude.meters());
    Serial.print(F(" Miles="));
    Serial.print(gps.altitude.miles());
    Serial.print(F(" KM="));
    Serial.print(gps.altitude.kilometers());
    Serial.print(F(" Feet="));
    Serial.println(gps.altitude.feet());
  }

  else if (gps.satellites.isUpdated())
  {
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());
  }

  else if (gps.hdop.isUpdated())
  {
    Serial.print(F("HDOP       Fix Age="));
    Serial.print(gps.hdop.age());
    Serial.print(F("ms raw="));
    Serial.print(gps.hdop.value());
    Serial.print(F(" hdop="));
    Serial.println(gps.hdop.hdop());
  }



/*

  if (gps.location.isUpdated())
  {

    valid_location = true;
    Lat = gps.location.lat();
    Long = gps.location.lng();
  
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);
  
  }


      // get time from GPS module
    else  if (gps.time.isValid())
      {
        Minute = gps.time.minute();
        Second = gps.time.second();
        Hour   = gps.time.hour();
      }

      // get date drom GPS module
   else   if (gps.date.isValid())
      {
        Day   = gps.date.day();
        Month = gps.date.month();
        Year  = gps.date.year();
      }

 else if (gps.date.isUpdated())
  {
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  }

 else if (gps.time.isUpdated())
  {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }


 else if (gps.speed.isUpdated())
  {
  
    Serial.print(F("SPEED      Fix Age="));
    Serial.print(gps.speed.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.speed.value());
    Serial.print(F(" Knots="));
    Serial.print(gps.speed.knots());
    Serial.print(F(" MPH="));
    Serial.print(gps.speed.mph());
    Serial.print(F(" m/s="));
    Serial.print(gps.speed.mps());
    Serial.print(F(" km/h="));
    Serial.println(gps.speed.kmph());
   

    gps_speed = gps.speed.kmph();
  }

else if (gps.course.isUpdated())
  {
    Serial.print(F("COURSE     Fix Age="));
    Serial.print(gps.course.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.course.value());
    Serial.print(F(" Deg="));
    Serial.println(gps.course.deg());
  }


 else  if (gps.altitude.isUpdated())
  {
 
    Serial.print(F("ALTITUDE   Fix Age="));
    Serial.print(gps.altitude.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.altitude.value());
    Serial.print(F(" Meters="));
    Serial.print(gps.altitude.meters());
    Serial.print(F(" Miles="));
    Serial.print(gps.altitude.miles());
    Serial.print(F(" KM="));
    Serial.print(gps.altitude.kilometers());
    Serial.print(F(" Feet="));
    Serial.println(gps.altitude.feet());
   
    alt = gps.altitude.meters();
  }

  
 else if (gps.satellites.isUpdated() )
  {
    num_sat = gps.satellites.value();
   
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());

  }


  else if (gps.hdop.isUpdated())
  {
    Serial.print(F("HDOP       Fix Age="));
    Serial.print(gps.hdop.age());
    Serial.print(F("ms raw="));
    Serial.print(gps.hdop.value());
    Serial.print(F(" hdop="));
    Serial.println(gps.hdop.hdop());
  }



  else  if (gps.location.isValid())
    {

    valid_location = true;
      
    }else{
          valid_location = false;
    }

    */

    
    if (totalGPGSVMessages.isUpdated())
    {
      for (int i=0; i<4; ++i)
      {
        int no = atoi(satNumber[i].value());
        // Serial.print(F("SatNumber is ")); Serial.println(no);
        if (no >= 1 && no <= MAX_SATELLITES)
        {
          sats[no-1].elevation = atoi(elevation[i].value());
          sats[no-1].azimuth = atoi(azimuth[i].value());
          sats[no-1].snr = atoi(snr[i].value());
          sats[no-1].active = true;
        }
      }
      
      int totalMessages = atoi(totalGPGSVMessages.value());
      int currentMessage = atoi(messageNumber.value());
      if (totalMessages == currentMessage)
      {
        Serial.print(F("Sats=")); Serial.print(gps.satellites.value());
        SerialBT.print("Sats=");SerialBT.print(gps.satellites.value());
        Serial.print(F(" Nums="));
          SerialBT.print(F(" Nums="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(i+1);
            Serial.print(F(" "));

            SerialBT.print(i+1);
            SerialBT.print(" ");
            
          }
        Serial.print(F(" Elevation="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].elevation);
            Serial.print(F(" "));
          }
        Serial.print(F(" Azimuth="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].azimuth);
            Serial.print(F(" "));
          }
        
        Serial.print(F(" SNR="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].snr);
            Serial.print(F(" "));
          }
        Serial.println();
        SerialBT.println();

        for (int i=0; i<MAX_SATELLITES; ++i)
          sats[i].active = false;
      }
    }


}
