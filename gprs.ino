//#include <HardwareSerial.h>
//#include <SoftwareSerial.h>
//#include <ESPSoftwareSerial.h>
#include <arduino.h>
#include <String.h>
#include <CircularBuffer.h>
#include <SoftwareSerial.h>

#define GPRS_RX 27
#define GPRS_TX 26
// buffer for 6 minutes data log
CircularBuffer<String, 99> contents;
CircularBuffer<String, 25> smsm;
CircularBuffer<String, 25> smsd;

CircularBuffer<String, 5> commands;
CircularBuffer<double, 5> ctimers;

SoftwareSerial gprs;//(33,32);

int smscount = 0;

// Create software serial object to communicate with SIM800L
// SoftwareSerial gprs;//(33,32);
// HardwareSerial gprs(1);
static const uint32_t GPRSBaud = 9600;
int listsms = 0;

int signalstat = 0;
int netstat = 0;
String ipaddr = "";
int inetstat = -1;
bool result = 0;
int atstat = 0;
int initstat = -1;
int idlesend = 0;
int idlenet = 0;
int errsend = 0;
int inetretry = 0;
int inetidle = 0;
int modemstat = 1;
int contsize = 0;
int modeminit = 1;
String mimei = "";
int lenresponse = 0;
String textresponse = "";
double lastchecksignal = 0;
int cfun = 0;

int firstdatasend = 0;

int statsend = 0;
int flaglistsms = 0;

// Modem status
// INIT -> first initial stat
// CONNECT_NET -> NETWORK CONNECTED
// CONNECT_INET -> Internet Connected
// SENDING_DATA  -> while send Data

String modemstatus = "INIT";

String atmsg = "";
String smsg = "";
String flagat = "";
String content = "";

double timernext = 0;
String nextproc = "";
String nextcmd = "";
int stillsend = 0;
double timercheck = 0;

double timernextinet = 0;
String nextprocinet = "";
String nextcmdinet = "";

double timernextrestart = 0;
double nextimers = 0;
String nextprocrestart = "";
String nextcmdrestart = "";
double lastcheck = 0;
double lastsend = 0;

int sendpause = 8;
int countersend = 0;

void init_gprs(void);
void gprs_handle(void);
void update_serial(void);
void processMsg(String);
void Send_data(void);
void data_init(void);
void modem_init(void);
void data_init1(void);
void data_init2(void);
void data_init3(void);
void data_init4(void);
void internet_init(void);
void initsend(void);
void setcontent(void);
void addcomtent(String);
void processCommands(void);
void processCommand(void);
void addcomtent(String);
int checknetstat(void);
int checkinetstat(void);
int checkbatstat(void);
int checkstatsend(void);
int contentsize(void);
int getlistsms(void);
int getimei(void);
String getreadsms(int);
void refreshsms(void);
String getheadersms(int);

String getresponse(void);
void setresponse(String);
int addcontentsend(String, int);

String tempstr = "";

int idleglobal = 0;
int batstat = 0;

String lastat = "";

double timerhandle = 0;
double timersend = 0;



String simei = "";
String sunique = "";

String webresponse = "";
String webtoken = "";
String messageid = "";

int isregistered = 0;
int smsread = 0;
int smsbody = 0;

String idstart = "";

void setcontent()
{
  content = contents.pop();
}

int contentsize()
{
  return contents.size();
}

void refreshsms()
{
  listsms = 0;
  gprs.println("AT+CMGF=1"); // hang up
  delay(200);
  gprs.println("AT+CMGL=\"ALL\""); // hang up
}

int getlistsms()
{
  return listsms;
}

String getresponse()
{
  return textresponse;
}

void setresponse(String str)
{
  textresponse = str;
}

String getimei(int types)
{
  // if (types==0)
  //   addCommand("AT+CGSN#GET_IMEI", millis() + 500);

  return mimei;
}

String getheadersms(int idx)
{
  if (listsms > 0)
  {
    // return smsm.pop();
    return smsm[idx];
  }
  return "";
}

int getflagsms()
{

  return flaglistsms;
}



void sendToWeb(String types, String params) {

  String cont = "param=" + types + "&device[]=" + simei;
  if ( params != "" ) {
    cont.concat(params);
  }

  cont.concat("&resp=");
  //contents.unshift(cont);
  Serial.print("Add Send to Web : ");
  //Serial.println(cont);
  //timerupdate = millis() + 1000;
  addcontentpush(cont);

  Serial.print("Total : ");
  Serial.println(contentsize());
  //setcontent();

}


String getreadsms(int idx)
{
  if (listsms > 0)
  {
    return smsd[idx];
  }
  return "";
}

void addcontent(String cont)
{
  contents.unshift(cont);
}

void addcontentpush(String cont)
{
  contents.push(cont);
}

int addcontentsend(String cont, int idx)
{
  idx++;
  if (idx == 1)
  {
    tempstr = "";
    tempstr.concat("dv=");
    tempstr.concat(simei);
    tempstr.concat("&st=");
    tempstr.concat(idstart);
    tempstr.concat("&cid=");
    tempstr.concat(sunique);
  }

  if (idx < 3)
  {

    if (tempstr != "")
      tempstr.concat("&");
    tempstr.concat(cont);
  }
  else
  {
    idx = 1;
    contents.unshift(tempstr);
    tempstr = "";
    tempstr.concat("dv=");
    tempstr.concat(simei);
    tempstr.concat("&st=");
    tempstr.concat(idstart);
    tempstr.concat("&cid=");
    tempstr.concat(sunique);
    tempstr.concat("&");
    tempstr.concat(cont);

    initsend();
  }

  return idx;
}

int checkbatstat()
{
  return batstat;
}

int checknetstat()
{
  return netstat;
}

int checkinetstat()
{
  return inetstat;
}

int checkstatsend()
{
  if (netstat < 1 || inetstat < 0)
    return 0;
  else
    return statsend;
}

/*void setup()
  {
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(115200);
  Serial.println("Initializing SIm800L");

  //Begin serial communication with Arduino and SIM800L
  //gprs.begin(9600);
  //gprs.begin(GPSBaud,SERIAL_8N1, 16, 17);
  gprs.begin(GPSBaud, SWSERIAL_8N1, 32, 33, false);

  //Serial.write(gprs);

  if (gprs) {
    initstat = 0;

    //  gprs.begin();

    Serial.println("Sim800L Initialized");
    delay(1000);
    gprs.println("AT");
  } else {

    Serial.println("Sim800L Initialized failed");
  }

  flagat = "INIT";

  }
*/

void initsend()
{
  if (contents.size() > 0 && stillsend == 0 && inetstat > 0)
  {
    stillsend = 1;
    // nextcmd = "AT";
    // nextproc = "INITS";
    // nextproc = "INITSEND";
    // timernext = millis() + 2000;
    content = contents.pop();
  }
  // Serial.print("Total Outstanding :");
  // Serial.println(contents.size());
}

void init_gprs()
{
  Serial.print("Start GPRS");
  gprs.begin(GPRSBaud, SWSERIAL_8N1, GPRS_RX, GPRS_TX, false);
  // nextcmd = "AT";
  // nextproc = "INIT";
  // nextproc = "INITSEND";
  // timernext = millis() + 2000;
  // flagat = "INIT";
  // gprs.println("AT");
  addCommand("AT#", millis() + 3000);
  // timernext = millis() + 3000;
  processCommands();

  addCommand("AT+GSN#GET_IMEI", millis() + 1000);
}

void updateSerial()
{

  while (Serial.available())
  {
    char cs = Serial.read();

    gprs.write(cs); // Forward what Serial received to Software Serial Port



    if (cs != '\r' && cs != '\n')
    {
      smsg.concat(cs);
    }
    else if (cs == '\n' && smsg.length() > 0)
    {
      processSerialMsg(smsg);
    }
    // if (counter==5)
    // u8g2log.print(cs);               // print to display
    // Serial.write(c);//Forward what Software Serial received to Serial Port

    // while (Serial.available() > 0) {
    //  c = Serial.read();      // read from Serial Monitor

    // }

  }

  while (gprs.available())
  {
    char c = gprs.read();
    if (c != '\r' && c != '\n')
    {
      atmsg.concat(c);
    }
    else if (c == '\n' && atmsg.length() > 0)
    {
      processMsg(atmsg);
    }

    // if (counter==5)
    // u8g2log.print(c);
    // u8g2log.print(c);
    Serial.write(c); // Forward what Software Serial received to Serial Port
  }
}

void processSerialMsg(String srmsg)
{

  Serial.print("Process Message = ");
  Serial.println(srmsg);


  if (srmsg == "MSG-TESTSEND")
  {
    // String smstext = "";
    String cont = "device[]=ACR001&cid[]=" + sunique + "&starts[]=";
    cont.concat(idstart);
    cont.concat("&lat[]=");
    cont.concat("000000");
    cont.concat("&lon[]=");
    cont.concat("000000");
    cont.concat("&speed[]=");
    cont.concat(100);
    cont.concat("&satused[]=");
    cont.concat(11);
    cont.concat("&hdop[]=");
    cont.concat(1201);
    cont.concat("&distance[]=");
    cont.concat(1200);
    cont.concat("&dir[]=");
    cont.concat(11);
    cont.concat("&time[]=");
    cont.concat("2012-01-01 00:00:00");
    cont.concat("&ct[]=");
    cont.concat("0.0");
    cont.concat("&bv[]=");
    cont.concat("0.0");
    cont.concat("&tmp[]=");
    cont.concat(temp);
    cont.concat("&acst[]=");
    cont.concat("0.0");
    cont.concat("&fanst[]=");
    cont.concat(fanst);
    cont.concat("&sgnlv[]=");
    cont.concat("0.0");
    cont.concat("&blv[]=");
    cont.concat("0.0");
    cont.concat("&gprsst[]=");
    cont.concat("0.0");
    // smstext.concat("device[]=ACR001&param=TEST&end=");
    // for (int j = 0; j < 20; j++) {
    //   smstext.concat("1234567890" + j);
    // }
    // contents.concat("device[]=ACR001&param=TEST&end=");
    contents.unshift(cont);
    // if (content=="")
    //  content = contents.pop();
    // triggerNextSend();
  }
  else if (srmsg == "MSG-MANUALTRIGGER")
  {
    triggerNextSend();
  }
  else if (srmsg == "MSG-RESTARTMODEM")
  {
    if (cfun == 0)
      return;
    Serial.println("message tp restart modem");
    addCommand("AT+CFUN=0#", millis() + 1000);
    cfun = 0;
    // timernext = millis() + 1000;
    // processCommands();
  }
  else if (srmsg == "MSG-RESTARTINET")
  {
    addCommand("AT+SAPBR=1,1#RESTART_INET", millis() + 1000);
    // timernext = millis() + 1000;
    // processCommands();
  }
  else if (srmsg.substring(0, 12) == "MSG-TESTSEND")
  {
    String param = getStringPartByNr(srmsg, ':', 1);
    contents.unshift(param);
  }
  else if (srmsg == "DEEP_SLEEP")
  {
    gprs.println("AT+CSCLK=2"); // hang up
    delay(2000);
    ESP.deepSleep(0);
  }

  // if (content == "")
  //   content = contents.pop();

  smsg = "";
}

String getStringPartByNr(String data, char separator, int index)
{
  int stringData = 0;   // variable to count data part nr
  String dataPart = ""; // variable to hole the return text

  for (int i = 0; i < data.length(); i++)
  { // Walk through the text one letter at a time
    if (data[i] == separator)
    {
      // Count the number of times separator character appears in the text
      stringData++;
    }
    else if (stringData == index)
    {
      // get the text when separator is the rignt one
      dataPart.concat(data[i]);
    }
    else if (stringData > index)
    {
      // return text and stop if the next separator appears - to save CPU-time
      return dataPart;
      break;
    }
  }
  // return text if this is the last part
  return dataPart;
}

void sendCommandAT(String command, String alias)
{
  flagat = alias;
  atmsg = "";
  // Serial.print("Commandnya=");
  // Serial.println(command);
  // u8g2log.print("Commandnya=");
  // u8g2log.println(command);
  if (command.substring(0, 2) != "AT" && command != "")
  {
    gprs.println("AT"); // hang up
  }
  else
    gprs.println(command); // hang up

  command = "";
}

void gprs_handle()
{
  updateSerial();

  if (netstat > 0)
    cfun = 1;
  else
    cfun = 0;

  if (millis() - timersend > 1000)
  {
    if (modemstat >= 1)
    {
      if (inetstat == 1 && netstat > 0)
        Send_data();

      if (inetidle >= 10 && netstat > 0)
      {
        inetidle = 0;
        // Serial.println("Restarting Inet");
        // nextprocinet = "RESTART_INET";
        // nextcmdinet = "AT+SAPBR=1,1";
        // timernextinet = millis() + 1000;
        addCommand("AT+SAPBR=1,1#RESTART_INET", millis() + 1000);
      }
    }

    timersend = millis();

    if (millis() - lastcheck > 20000)
    {
      // there is not activity during 10 seconds
      lastcheck = millis();

      addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 500);
      // timernext = millis() + 500;
      // processCommands();
    }

    if (mimei == "")
    {
      addCommand("AT+GSN#GET_IMEI", millis() + 1000);
    }
    else
    {
      // Serial.println("Imei="+mimei);
    }
  }

  if (millis() - timernext > 1000)
  {
    timernext = millis();
    if (nextcmd != "")
    {
      if (millis() - nextimers > 0)
      {
        //  timernext = millis() + 4000;
        sendCommandAT(nextcmd, nextproc);
        nextproc = "";
        nextcmd = "";
        processCommands();
      }
    }
    else
    {
      if (commands.size() > 0)
        processCommands();
    }

    lastat = flagat;
  }

  if (millis() - timernextinet > 100)
  {
    if (nextprocinet != "")
    {
      //  timernext = millis() + 4000;
      sendCommandAT(nextcmdinet, nextprocinet);
      nextprocinet = "";
      nextcmdinet = "";
    }

    lastat = flagat;
  }

  if (millis() - timernextrestart > 100)
  {
    if (nextprocrestart != "")
    {
      //  timernext = millis() + 4000;
      // sendCommandAT(nextcmdrestart, nextprocrestart);
      nextprocrestart = "";
      nextcmdrestart = "";
    }

    lastat = flagat;
  }

  if (millis() - timercheck > 5000)
  {
    if (modemstat == 3)
    {
      // Serial.println("Check NET PROC");
      // if (nextproc == ""){
      //  nextproc = "CHECK_SIGNAL";
      // gprs.println("AT+CSQ");
      //  addCommand("AT+CSQ#check");
      //  timernext = millis() + 1000;
      //  processCommands();
    }
    timercheck = millis();
    //}
  }

  if (millis() - timerhandle > 1000)
  {
    timerhandle = millis();

    Serial.print("flag at=");
    Serial.print(flagat);

    Serial.print(", idle=");
    Serial.print(idlesend);

    Serial.print(", idlenet=");
    Serial.print(idlenet);

    Serial.print(", idleglobal=");
    Serial.println(idleglobal);

    Serial.print(", idlesend=");
    Serial.println(idlesend);


    if (lastat == flagat && flagat != "" && flagat != "NORMAL_MODE" && flagat != "RESTART_MODE" && flagat != "CHECK_SIGNAL")
    {
      idleglobal++;
    }
    else
    {
      idleglobal = 0;
    }

    if (netstat < 1)
    {
      idlenet++;
    }
    else
    {
      idlenet = 0;
    }

    if (idlenet > 20)
    {
      idlenet = 0;
      if (cfun == 0)
      {
        addCommand("AT+CFUN=1#", millis() + 500);
        return;
      }
      Serial.println("restart modem");
      // restart modem

      cfun = 0;
      addCommand("AT+CFUN=0#", millis() + 500);
    }

    if (idleglobal > 10)
    {
      // idlenet = 0;
      idleglobal = 0;
      initstat = 1;
      // nextproc = "NORMAL_MODE";
      // nextcmd = "AT+CFUN=0";
      // timernext = millis() + 2000;
    }

    flaglistsms++;

    if (flaglistsms > 100)
      flaglistsms = 5;
  }
}

/*
  void loop()
  {
  gprs_handle();
  }
*/

String buffsms;
int streamsms = 0;

void addCommand(String comms, double timers)
{
  commands.unshift(comms);
  ctimers.unshift(timers);
  Serial.println("Add Commands:" + comms);
  Serial.print("delay for:");
  Serial.println(millis() - timers);
}

void triggerNextSend()
{
  if (flagat != "SEND_DATA" and contents.size() > 0 and inetstat > 0 and netstat > 0 and content == "")
  {
    countersend = 0;
    timersend = millis() + 500;
    content = contents.pop();
  }
  else if (flagat != "SEND_DATA" and contents.size() == 0 and inetstat > 0 and netstat > 0)
  {
    if (millis() - lastchecksignal > 5000)
      addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 5000);
    // timernext = millis() + 5000;
    // processCommands();
  }
}

void clearCommand()
{
  while (commands.size() > 0)
  {
    commands.pop();
  }
}

void processCommands()
{
  if (commands.size() > 0 && nextcmd == "")
  {
    String comms = commands.pop();
    nextimers = ctimers.pop();

    String coms = getStringPartByNr(comms, '#', 0);
    String procs = getStringPartByNr(comms, '#', 1);
    coms.replace("#", "");
    procs.replace("#", "");
    nextproc = procs;
    nextcmd = coms;
    Serial.println("nextproc :" + procs);
    Serial.println("nextcommand :" + coms);


    
    // timernext = millis() + 1000;
  }
}

void processMsg(String msg)
{
  String mssg = msg;
  Serial.println("Message :" + mssg);
  // Serial.println("Flag ATs :" + flagat);
  // Serial.println("initstat :" + initstat);
  atmsg = "";

  // Process based on Replying

  if (flagat == "INIT_HTTP")
  {
    if (mssg.substring(0, 2) == "OK")
    {
      flagat = "START_HTTP";
    }
    else
    {
      flagat = "";
    }
    return;
  }

  if (mssg.substring(0, 6) == "+CMGL:")
  {
    streamsms = 1;
    if (listsms == 0)
    {
      while (!smsm.isEmpty())
      {
        // pick the correct one
        // delete smsm.pop();
        // free(smsm.pop());
        smsm.pop();
      }
      smsm.clear();

      while (!smsd.isEmpty())
      {
        // pick the correct one
        // delete smsm.pop();
        // free(smsm.pop());
        smsd.pop();
      }
      smsd.clear();

      buffsms = "";
    }

    String from = getStringPartByNr(mssg, ',', 2);

    from.replace('"', '\0');

    smsm.push(from);

    if (buffsms != "")
    {
      smsd.push(buffsms);
    }
    listsms++;
    flaglistsms = 0;
    buffsms = "";
  }

  if (flagat == "GET_IMEI")
  {
    flagat = "";
    mimei = mssg;
  }

  if (streamsms == 1 && flaglistsms <= 1 && mssg.substring(0, 6) != "+CMGL:")
  {
    flaglistsms = 0;
    buffsms.concat(mssg);
    // smsd[listsms-1] = buffsms;
    // Serial.print("body: [");
    // Serial.print(buffsms);
    //  Serial.println("]");
  }

  if (buffsms != "" && streamsms == 1 && mssg.substring(0, 2) == "OK" && mssg.charAt(3) == '\n')
  {
    streamsms = 0;
    buffsms = "";
    smsd.push(buffsms);
  }

  if (streamsms == 1 && flaglistsms > 3)
  {
    streamsms = 0;
  }

  if (streamsms == 1)
  {
    return;
  }

  if (lenresponse > 0 and mssg.length() == lenresponse)
  {
    lenresponse = 0;
    mssg.trim();
    textresponse = mssg;
    // Serial.println("Response=" + textresponse);
  }

  if (mssg.substring(0, 5) == "+CSQ:" && flagat != "CHECK_SIGNAL")
  {
    // Checking signal Quality
    String sign = getStringPartByNr(mssg, ',', 0);
    sign.replace("+CSQ:", "");
    sign.trim();
    // Serial.println("");
    // Serial.print("Sign: ");
    // Serial.println(sign);
    netstat = sign.toInt();

    // Serial.print("Net Stat: ");
    // Serial.println(netstat);

    if (netstat < 1)
    {
      // inetstat = 0;
      initstat = 1;
      // nextproc = "CHECK_SIGNAL";
      // nextcmd = "AT+CSQ";
      // timernext = millis() + 2000;
      addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 2000);
      lastchecksignal = millis();
      // timernext = millis() + 2000;
      // processCommands();
      //  modem_init();
    }
    else
    {
      // if (inetstat<1){
      if (initstat < 2)
      {
        // initstat = 2;
        // nextproc = "CHECK_SIGNAL";
        // nextcmd = "AT+CSQ";
        // timernext = millis() + 2000;

        addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 2000);
        lastchecksignal = millis();
        // timernext = millis() + 2000;
        // processCommands();
        return;
      }
      // Serial.print("Checking inet interval");
      // gprs.println("AT+SAPBR=2,1");
      //   addCommand("AT+SAPBR=2,1#");
      //   processCommands();
      // }
    }
  }
  else if (mssg.substring(0, 7) == "+SAPBR:")
  {
    // Checking Internet Status
    String stats = getStringPartByNr(mssg, ',', 1);

    if (stats.toInt() == 1)
    {
      inetstat = 1;
      inetidle = 0;
    }
    else
    {
      if (inetstat >= 0)
        inetstat = 0;
      // initstat = 1;
      // inetstat = 0;
      //   inetstat = 0;
      inetidle++;
    }

    // timernext = millis() + 500;
    // processCommands();
    // return;
    // gprs.println("AT+CBC");
    // addCommand("AT+CBC", millis() + 2000);

    lastcheck = millis();
    // Checking Battery Status
    // String stats = getStringPartByNr(mssg, ',', 1);
    // ipaddr.replace(""","");
    // batstat = stats.toInt();
    // processCommands();
    triggerNextSend();
    if (millis() - lastsend > 8000)
    {
      lastsend = millis();
      // Serial.println("Add Check Message");
      String cont = "&starts[]=";
      cont.concat(idstart);
      cont.concat("&lat[]=");
      cont.concat("000000");
      cont.concat("&lon[]=");
      cont.concat("000000");
      cont.concat("&speed[]=");
      cont.concat(100);
      cont.concat("&satused[]=");
      cont.concat(11);
      cont.concat("&hdop[]=");
      cont.concat(1201);
      cont.concat("&distance[]=");
      cont.concat(1200);
      cont.concat("&dir[]=");
      cont.concat(11);
      cont.concat("&time[]=");
      cont.concat("2012-01-01 00:00:00");
      cont.concat("&ct[]=");
      cont.concat("0.0");
      cont.concat("&bv[]=");
      cont.concat("0.0");
      cont.concat("&tmp[]=");
      cont.concat(temp);
      cont.concat("&acst[]=");
      cont.concat("0.0");
      cont.concat("&fanst[]=");
      cont.concat(fanst);
      cont.concat("&sgnlv[]=");
      cont.concat("0.0");
      cont.concat("&blv[]=");
      cont.concat("0.0");
      cont.concat("&gprsst[]=");
      cont.concat("0.0");
      sendToWeb("CHECK_MESSAGE", cont);
    }
    if (inetstat > 0)
      return;
  }
  else if (mssg.substring(0, 5) == "+CBC:")
  {
    /*
      lastcheck = millis();
      //Checking Battery Status
      String stats = getStringPartByNr(mssg, ',', 1);
      //ipaddr.replace(""","");
      batstat = stats.toInt();
      //processCommands();
      triggerNextSend();
      if (millis() - lastsend > 8000) {
      lastsend = millis();
      //Serial.println("Add Check Message");
      String cont = "&starts[]=";
      cont.concat(idstart);
      cont.concat("&lat[]=");
      cont.concat("000000");
      cont.concat("&lon[]=");
      cont.concat("000000");
      cont.concat("&speed[]=");
      cont.concat(100);
      cont.concat("&satused[]=");
      cont.concat(11);
      cont.concat("&hdop[]=");
      cont.concat(1201);
      cont.concat("&distance[]=");
      cont.concat(1200);
      cont.concat("&dir[]=");
      cont.concat(11);
      cont.concat("&time[]=");
      cont.concat("2012-01-01 00:00:00");
      cont.concat("&ct[]=");
      cont.concat("0.0");
      cont.concat("&bv[]=");
      cont.concat("0.0");
      cont.concat("&tmp[]=");
      cont.concat(temp);
      cont.concat("&acst[]=");
      cont.concat("0.0");
      cont.concat("&fanst[]=");
      cont.concat(fanst);
      cont.concat("&sgnlv[]=");
      cont.concat("0.0");
      cont.concat("&blv[]=");
      cont.concat("0.0");
      cont.concat("&gprsst[]=");
      cont.concat("0.0");
      sendToWeb("CHECK_MESSAGE", cont);
      }
      return ;
    */
  }
  else if (mssg.substring(0, 10) == "+HTTPREAD:")
  {
    // Response from Web
    String resp = getStringPartByNr(mssg, ':', 1);
    resp.replace(" ", "");
    lenresponse = resp.toInt();
    //gprs.println("AT+HTTPTERM");
    triggerNextSend();
  }
  else if (mssg.substring(0, 12) == "+CPIN: READS")
  {
    // first stat when modem restarted
    modeminit = 1;

    clearCommand();
    addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 2000);
    lastchecksignal = millis();
    // timernext = millis() + 2000;
    // processCommands();
    return;
  }
  else if (mssg.substring(0, 6) == "+CMGL:")
  {
    // Checking Internet Status
    String stats = getStringPartByNr(mssg, ',', 1);
    // Serial.print("Messagenya : [ ");
    // Serial.print(mssg);
    // Serial.println("]");
  }
  else if (mssg.substring(0, 10) == "Call Ready")
  {
    // first stat when modem restarted
    modeminit = 1;
    errsend = 0;
    flagat = "";
    inetstat = -1;
    initstat = 0;
    netstat = 0;
    inetretry = 0;
    clearCommand();
    addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 2000);
    lastchecksignal = millis();
    // timernext = millis() + 2000;
    // processCommands();
    return;
  }
  else if (mssg.substring(0, 16) == "+CPIN: NOT READY")
  {
    // modem has been restart
    // try to start again
    inetstat = -1;
    netstat = 0;
    initstat = 0;

    addCommand("AT+CFUN=1#", millis() + 500);
    // timernext = millis() + 500;
    // processCommands();
    return;
    // Serial.print("Message last=");
    // Serial.println(mssg);
  }

  if (initstat <= 0)
  {
    if (mssg == "OK")
    {
      // First Message Captured
      initstat = 1;
      // nextproc = "CHECK_SIGNAL";
      // nextcmd = "AT+CSQ";
      // timernext = millis() + 2000;

      addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 500);
      lastchecksignal = millis();
      // processCommands();
    }
  }
  else
  {

    if (flagat == "CHECK_SIGNAL")
    {
      if (mssg.substring(0, 5) == "+CSQ:")
      {
        flagat = "";

        String sign = getStringPartByNr(mssg, ',', 0);
        sign.replace("+CSQ:", "");
        sign.trim();
        // Serial.println("");
        // Serial.print("Sign: ");
        // Serial.println(sign);
        netstat = sign.toInt();

        // Serial.print("Net Stat: ");
        // Serial.println(netstat);
      }
      else if (mssg.substring(0, 11) == "+CME ERROR:")
      {
        flagat = "";
      }

      if (mssg.substring(0, 5) == "+CSQ:")
      {
        // Serial.print("Call Again=");
        // Serial.println(netstat);
        if (netstat > 0 && inetstat <= 0)
        {
          // sendCommandAT("AT+SAPBR=2,1", "CHECK_INET");
          //  Serial.print("Check INET DULU");
          // nextproc = "CHECK_INET";
          // nextcmd = "AT+SAPBR=2,1";
          // timernext = millis() + 2000;

          addCommand("AT+SAPBR=2,1#CHECK_INET", millis() + 2000);
          // timernext = millis() + 2000;
          // processCommands();
          return;
        }
        else if (netstat < 1)
        {

          idlenet++;
          // flagat = "";
          // Serial.print("Check idlenet=");
          // Serial.println(idlenet);
          if (idlenet < 5)
          {
            // gprs.println("AT+CFUN=1");
            // Serial.println("Call check signal again");
            // nextproc = "CHECK_SIGNAL";
            // nextcmd = "AT+CSQ";
            // timernext = millis() + 3000;

            addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 500);
            lastchecksignal = millis();
            // timernext = millis() + 500;
            // processCommands();
            return;
          }
          else
          {

            // oidlenet = 0;
            if (cfun == 0)
              return;
            // gprs.println("AT+CFUN=0");
            Serial.println("restart modem trigger idlenet");
            addCommand("AT+CFUN=0#", millis() + 500);
            cfun = 0;
            // Serial.println("Restarting Modem#");
            // nextprocinet = "RESTART MODEM";
            //  nextcmdinet = "AT+CFUN=1";
            // timernextinet = millis() + 2000;
            // addCommand("AT+CFUN=0#", millis() + 500);
            // timernext = millis() + 500;
            // processCommands();
            return;
          }
          // sendCommandAT("AT+CSQ", "CHECK_SIGNAL");
        }
        else
        {

          addCommand("AT+SAPBR=2,1#", millis() + 500);
          // timernext = millis() + 500;
          // processCommands();
          return;
          // triggerNextSend();
        }
      }
    }
    else if (flagat == "STAT_SIGNAL")
    {
      if (mssg.substring(0, 5) == "+CSQ:")
      {
        flagat = "";

        String sign = getStringPartByNr(mssg, ',', 0);
        sign.replace("+CSQ:", "");
        sign.trim();
        // Serial.println("");
        // Serial.print("Sign: ");
        // Serial.println(sign);
        netstat = sign.toInt();

        // Serial.print("Net Stat: ");
        // Serial.println(netstat);
      }
    }
    else if (flagat == "RESTART_MODE")
    {
      flagat = "";
      idlenet = 0;
      inetstat = 0;
      netstat = 0;
      // nextproc = "CHECK_SIGNAL";
      // nextcmd = "AT+CSQ";
      // timernext = millis() + 4000;
      addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 500);
      // processCommands();
    }
    else if (flagat == "MODEM_INIT1")
    {
      flagat = "";
      // nextcmd = "AT+CNMI=2,2,0,0,0";
      // nextproc =  "MODEM_INIT2";
      // timernext = millis() + 1000;

      addCommand("AT+CNMI=2,2,0,0,0#MODEM_INIT2", millis() + 1000);
      // timernext = millis() + 1000;
      // processCommands();
      // addCommand("modem_init3#MODEM_INIT3");
      // processCommands();
    }
    else if (flagat == "MODEM_INIT2")
    {
      data_init();
      // flagat = "";
      // nextcmd = "AT+CNMI=2,2,0,0,0"";
      // nextproc =  "MODEM_INIT2";
      // timernext = millis() + 1000;
    }
    else if (flagat == "DATA_INIT1")
    {
      flagat = "";
      data_init1();
    }
    else if (flagat == "DATA_INIT2")
    {
      flagat = "";
      data_init2();
    }
    else if (flagat == "DATA_INIT3")
    {
      flagat = "";
      data_init3();
    }
    else if (flagat == "DATA_INIT4")
    {
      flagat = "";
      data_init4();
    }
    else if (flagat == "INTERNET_INIT")
    {
      flagat = "";
      internet_init();
      // flagat = "";
      // nextcmd = "AT+CNMI=2,2,0,0,0"";
      // nextproc =  "MODEM_INIT2";
      // timernext = millis() + 1000;
      // flagat = "";
      // nextcmd = "AT+CNMI=2,2,0,0,0"";
      // nextproc =  "MODEM_INIT2";
      // timernext = millis() + 1000;
    }
    else if (flagat == "NORMAL_MODE")
    {
      flagat = "";
      idlenet = 0;
      inetstat = 0;
      netstat = 0;
      // Serial.print("Restarting");
      // nextprocrestart = "RESTART_MODE";
      // nextcmdrestart = "AT+CFUN=1";
      // timernextrestart = millis() + 1000;
      clearCommand();
      addCommand("AT+CFUN=1#RESTART_MODE", millis() + 1000);
      // processCommands();
    }
    else if (flagat == "CHECK_INET")
    {
      // Serial.println("Check inet");

      if (mssg.substring(0, 7) == "+SAPBR:")
      {

        flagat = "";
        String stats = getStringPartByNr(mssg, ',', 1);
        // ipaddr.replace(""","");

        int fullproc = 0;

        if (inetstat < 0)
          fullproc = 1;

        if (stats.toInt() == 1)
        {
          inetstat = 1;
        }
        else
        {
          inetstat = 0;
        }

        // Serial.print("Internet Stat: ");
        // Serial.println(inetstat);
        // Serial.print("Ip Address: ");
        // Serial.println(ipaddr);

        if (inetstat == 0 && netstat > 0)
        {
          if (fullproc == 1)
          {
            modeminit = 0;
            inetstat = 0;
            modem_init();
          }
          else
            data_init4();
          // data_init();
          // updateSerial();
          // internet_init();
          //  sendCommandAT("AT+SAPBR=2,1", "CHECK_INET");
        }
      }
      else if (mssg == "ERROR")
      {
        flagat = "";
      }
    }
    else if (flagat == "STAT_INET")
    {
      // Serial.println("In Check inet");

      if (mssg.substring(0, 7) == "+SAPBR:")
      {

        flagat = "";
        String stats = getStringPartByNr(mssg, ',', 1);
        // ipaddr.replace(""","");
        if (stats.toInt() == 1)
        {
          inetstat = 1;
        }
        else
        {
          inetstat = 0;
        }

        // Serial.print("Internet Stat: ");
        // Serial.println(inetstat);
        // Serial.print("Ip Address: ");
        // Serial.println(ipaddr);

        if (inetstat == 0 && netstat > 0)
        {
          modem_init();
          // data_init();
          // updateSerial();
          // internet_init();
          //  sendCommandAT("AT+SAPBR=2,1", "CHECK_INET");
        }
      }
      else if (mssg == "ERROR")
      {
        flagat = "";
      }
    }
    else if (flagat == "SEND_DATA" || mssg.substring(0, 12) == "+HTTPACTION:")
    {

      if (mssg.substring(0, 12) == "+HTTPACTION:")
      {
        String stats = getStringPartByNr(mssg, ',', 1);
        String lens = getStringPartByNr(mssg, ',', 2);

        String conts = "";
        if (stats == "200")
        {

          lastcheck = millis();
          flagat = "";
          countersend++;
          errsend = 0;
          firstdatasend = 1;
          modemstat = 3;
          statsend = 2;

          if (content.substring(content.length() - 5) == "resp=" || lens == "45" // NEW_MESSAGE  or #STAT:UNREG
             )
          {
            // addCommand("AT+HTTPREAD#", millis() + 10);
            // webresponse = "";
            content = "";
            flagat = "";
            lenresponse = 0;
            gprs.println("AT+HTTPREAD");
            return;
          }
          else
          {
            content = "";
            flagat = "";
            inetretry = 0;
            //gprs.println("AT+HTTPTERM");
          }

          content = "";
          Serial.println("Data Sent");
          flagat = "";
          inetretry = 0;

          int count = 0;

          // Serial.print("countersend = ");
          // Serial.println(countersend);
          // Serial.print("sendpause = ");
          // Serial.println(sendpause);
          // Serial.print("Outstanding = ");
          // Serial.println(contents.size());

          if (countersend < sendpause)
          {
            if (contents.size() > 0)
            {
              conts = contents.pop();

              /*while (contents.size() > 0)
                {
                count++;

                if (count > 1)
                  break;

                if (conts != "")
                  conts.concat("&");

                conts.concat(contents.pop());

                }
              */
              content = conts;
              timersend = millis() + 1;
              return;
            }
            else
            {
              stillsend = 0;
              // addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 3000);
              // timernext = millis() + 3000;
              // processCommands();
              triggerNextSend();
              return;
            }
          }
          else if (countersend <= sendpause)
          {
            flagat = "";
            // pause first send data process
            addCommand("AT+CSQ#CHECK_SIGNAL", millis() + 500);
            // timernext = millis() + 500;
            // processCommands();
            return;
          }
        }
        else if (stats.substring(0, 2) == "60")
        {
          errsend++;
          flagat = "";
          Serial.print("Data Not Sent Status : ");
          Serial.println(stats);
          //gprs.println("AT+HTTPTERM");
        }
        else
        {
          // errsend++;
          flagat = "";
          // timersend = millis() + 500;
          // stillsend = 0;
        }
      }
      else if (mssg.substring(0, 11) == "+CME ERROR:")
      {
        errsend++;
        flagat = "";
        Serial.print("Error:");
        Serial.println(mssg);
        //gprs.println("AT+HTTPTERM");
        // flagat = "";
        timersend = millis() + 7000;
      }
    }
  }

  if (errsend >= 10)
  {
    Serial.println("Error more than 10 times, Restart Process !");
    flagat = "";
    inetstat = 0;
    errsend = 0;

    if (inetretry < 5)
    {
      inetretry++;
      gprs.println("AT+SAPBR=0,1");
      Serial.println("Restarting Internet only");
      // nextprocinet = "RESTART_INET";
      // nextcmdinet = "AT+SAPBR=1,1";
      // timernextinet = millis() + 1000;

      addCommand("AT+SAPBR=1,1#RESTART_INET", millis() + 1000);
      // timernext = millis() + 1000;
      // processCommands();
      return;
    }
    else
    {
      if (cfun == 0)
        return;
      Serial.println("Restarting Modem");
      errsend = 0;
      flagat = "";
      inetstat = 0;
      initstat = 1;
      netstat = 0;
      inetretry = 0;
      clearCommand();
      cfun = 0;
      addCommand("AT+CFUN=0#", millis() + 500);
      // timernext = millis() + 500;
      // processCommands();
      // gprs.println("AT+CFUN=0");
      // nextprocinet = "RESTART MODEM";
      // nextcmdinet = "AT+CFUN=1";
      // timernextinet = millis() + 1000;
      // addCommand("AT+SAPBR=1,1#RESTART_MODEM");
      // processCommands();
    }
  }
}

void modem_init()
{
  // Serial.println("Modem init.....");
  flagat = "";
  nextcmd = "AT+CMGF=1";
  nextproc = "MODEM_INIT1";
  timernext = millis() + 1000;
}

void data_init()
{
  // Serial.println("Data init ...");
  flagat = "";
  // nextcmd = "data_init1";
  //  nextproc = "DATA_INIT1";
  //  timernext = millis() + 1000;

  addCommand("data_init1#DATA_INIT1", millis() + 1000);
  // timernext = millis() + 1000;
  // processCommands();
  // gprs.write(0x0d);
  // gprs.write(0x0a);
  //  delay(1000); ;

  // delay(1000);
}

void data_init1()
{
  // gprs.println("AT+CPIN?");
  // delay(1000);
  flagat = "";
  gprs.print("AT+SAPBR=3,1");
  gprs.write(',');
  gprs.write('"');
  gprs.print("contype");
  gprs.write('"');
  gprs.write(',');
  gprs.write('"');
  gprs.print("GPRS");
  gprs.write('"');
  gprs.println("");

  // nextcmd = "data_init2";
  // nextproc = "DATA_INIT2";

  addCommand("data_init2#DATA_INIT2", millis() + 1000);
  // timernext = millis() + 1000;
  // processCommands();
}

void data_init2()
{
  flagat = "";
  gprs.print("AT+SAPBR=3,1");
  gprs.write(',');
  gprs.write('"');
  gprs.print("APN");
  gprs.write('"');
  gprs.write(',');
  gprs.write('"');
  //------------APN------------//
  gprs.print("3gprs"); // APN Here
  //--------------------------//
  gprs.write('"');
  // gprs.write(0x0d);
  // gprs.write(0x0a);
  gprs.println("");

  nextcmd = "data_init3";
  nextproc = "DATA_INIT3";
  timernext = millis() + 1000;

  addCommand("data_init3#DATA_INIT3", millis() + 1000);
  // timernext = millis() + 1000;
  // processCommands();
}

void data_init3()
{
  flagat = "";
  gprs.print("AT+SAPBR=3,1");
  gprs.write(',');
  gprs.write('"');
  gprs.print("USER");
  gprs.write('"');
  gprs.write(',');
  gprs.write('"');
  gprs.print("3gprs");
  gprs.write('"');
  // gprs.write(0x0d);
  // gprs.write(0x0a);
  gprs.println("");
  // nextcmd = "data_init4";
  // nextproc = "DATA_INIT4";
  // timernext = millis() + 1000;

  addCommand("data_init4#DATA_INIT4", millis() + 1000);
  // timernext = millis() + 1000;
  // processCommands();
}

void data_init4()
{
  flagat = "";
  gprs.print("AT+SAPBR=1,1");
  // gprs.write(0x0d);
  // gprs.write(0x0a);
  gprs.println("");

  nextcmd = "internet_init";
  nextproc = "INTERNET_INIT";
  timernext = millis() + 1000;
}

void internet_init()
{
  flagat = "";
  Serial.println("HTTP  init.....");
  // delay(1000);
  gprs.println("AT+HTTPINIT");
  // // delay(1000); delay(1000);
  gprs.print("AT+HTTPPARA=");
  gprs.print('"');
  gprs.print("CID");
  gprs.print('"');
  gprs.print(',');
  gprs.println('1');

  gprs.print("AT+HTTPPARA=");
  gprs.print('"');
  gprs.print("CONTENT");
  gprs.print('"');
  gprs.print(',');
  gprs.print('"');
  gprs.print("application/json");
  gprs.println('"');

  // AT+HTTPPARA="CONTENT","application/json"

  // nextproc = "CHECK_INET";
  // nextcmd = "AT+SAPBR=2,1";
  // timernext = millis() + 2000;
  // addCommand("AT+SAPBR=2,1#CHECK_INET", millis() + 1000);
  // timernext = millis() + 1000;
  // processCommands();
  modemstat = 2;
  return;
}

void Send_data()
{
  // Serial.println("Timer Call Send Data");

  if (flagat != "SEND_DATA" && flagat != "INIT_HTTP" && flagat != "START_HTTP")
  {
    // flagat = "INIT_HTTP";
    // gprs.println("AT+HTTPINIT");
    // return;
  }

  if (flagat != "SEND_DATA" && flagat != "START_HTTP" && flagat != "RESTART_MODEM" && inetstat == 1 && netstat > 0 && content != "")
  {

    idlesend = 0;
    statsend = 0;
    flagat = "SEND_DATA";
    idlesend = 0;
    Serial.print("Total Outstanding :");
    Serial.println(contents.size());

    // gprs.println("AT+HTTPINIT");

    Serial.println("Send data....:"); // + content);
    gprs.print("AT+HTTPPARA=");
    gprs.print('"');
    gprs.print("URL");
    gprs.print('"');
    gprs.print(',');
    gprs.print('"');
    gprs.print("http:");
    gprs.print('/');
    gprs.print('/');
    //-----------------------Your API Key Here----------------------//
    // Replace xxxxxxxxxxx with your write API key.
    gprs.print("apitrack.kitabisa.web.id");
    gprs.print("/trackdevice.php?");
    gprs.print(content);
    //---------------------------------------------------------------//
    gprs.print(millis()); //>>>>>>  variable 1 (temperature)
    // gprs.print("&field2=");
    // gprs.print(humi); //>>>>>> variable 2 (Humidity)
    // gprs.write(0x0d);
    // gprs.write(0x0a);
    gprs.println("\"");

    // Serial.println("Sending data");
    lastsend = millis();
  }
  else
  {
    idlesend++;

    if (idlesend <= 1 && flagat == "SEND_DATA")
    {
      statsend = 1;
      gprs.println("AT+HTTPACTION=0");

      // Serial.print("Data Sent");
    }
    else if (idlesend == 10 && flagat == "SEND_DATA" && netstat < 1)
    {
      idlesend = 0;
      // there is something wrong, restart modem
      // Serial.println("Set to air plane mode");
      // gprs.println("AT+CFUN=0");
      idlesend = 0;
      idlenet = 0;
      netstat = 0;
    }
    else if (flagat == "")
    {
      idlesend = 0;
    }
  }
}
