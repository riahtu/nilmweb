//==================================================
// SW-14-ethShield-temperature.ino
// 2016-04-08
//==================================================
int app_id = 14;
//==================================================
// Configuration
//==================================================
int g_debug              = 0;
const char* g_clientName = "SW-14";
const char* g_confServer = "sercon.simuino.com";

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int g_device_delay       = 3;
//==================================================
#define NFLOAT 2  // No of decimals i float value
#define NSID  2   // No of SIDs
#define SID1 901  // Temp 1 and Control SID
#define SID2 902  // Temp 2
#define SID3 903  // Temp 3
#define SID4 904  // Temp 4
#define SID5 905  // 
#define SID6 906
#define SID7 907
#define SID8 908

#define greenLed  13
#define yellowLed 12
#define redLed     5

#define MAX_SID 8
char g_server[120]; 
int  g_sids[10] = {NSID,SID1,SID2,SID3,SID4,SID5,SID6,SID7,SID8};
char g_sIp[80];
char g_rbuf[4000];
int  g_device_state = 0;
  
// Arduino-RPi protocol
#define NABTON_DATA     1 
#define NABTON_LATEST   2 
#define NABTON_MAILBOX  3 

#define S_STARTED    1
#define S_NO_NETWORK 2
#define S_NETWORK    3
#define S_INTERNET   4
#define S_CONFIGURED 5
#define S_RUNNING    6 
//=================================================
//
// D0 RX used for serial communication to server (Raspberry Pi)
// D1 TX used for serial communication to server (Raspberry Pi)
// D2 LED Internet
// D3 LED - Serial data received
// D4 IR Data
// D5 LED - Serial data sent
// D6 DIR Stepper
// D7 STEP Stepper
// D8 SLEEP Stepper
// D9 One Wire Data
// D10  RX Bluetooth device
// D11  TX Bluetooth device
// D12  LED Device status
// D13  LED Device Configured
//
// A0
// A1
// A2
// A3
// A4 SDA I2C OLED
// A5 SCL I2C OLED

// MEGA
// D20 SDA I2C OLED
// D21 SCL I2C OLED
// 
//=================================================
#include <stdio.h>
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
//#include <IRremote.h>
#include <U8glib.h>

//==================================================
// OLED I2C
//==================================================

//U8GLIB_SSD1306_128X64 u8g(13, 11, 10, 9);// SW SPI protocol(4 pins): SCK = 13, MOSI = 11, CS = 10, A0 = 9 
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE); // Small display I2C protocol (2 pins)
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE); // Large display
char dl[16][16],dm[16][16],dr[16][16];
//=================================================
// Ethernet
//=================================================
IPAddress ip(192, 168, 0, 99); // If no DHCP
//IPAddress ip;
IPAddress ipAddress;
EthernetClient client;
//=================================================
// One Wire
//=================================================

#define ONE_WIRE_BUS 9
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress device[MAX_SID];
int nsensors = 0;

//=================================================
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
//=================================================
{
asm volatile ("  jmp 0");  
}  
//=================================================
void NB_oledDraw() 
//=================================================
{
 u8g.firstPage();  
  do {
        draw();
  } while( u8g.nextPage() ); 
}

//=================================================
void draw()
//=================================================
{
  // Horizontal pixels: 0 - 120
  // Vertical pixels: 0 - 63
  u8g.setFont(u8g_font_6x10);
  //u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  /*u8g.drawStr( 0, 1, ".....");
  u8g.drawStr( 45, 1, ".....");
  u8g.drawStr( 90, 1, ".....");
  
  u8g.drawStr( 0, 63, "_____");
  u8g.drawStr( 45,63, "_____");
  u8g.drawStr( 90,63, "_____");*/
  
  u8g.drawStr( 0, 10, dl[1]);
  u8g.drawStr( 0, 27, dl[2]);
  u8g.drawStr( 0, 45, dl[3]);
  u8g.drawStr( 0, 62, dl[4]);

  u8g.drawStr( 45, 10, dm[1]);
  u8g.drawStr( 99, 27, dm[2]);
  u8g.drawStr( 99, 45, dm[3]);
  u8g.drawStr( 45, 62, dm[4]);

  u8g.drawStr( 100, 10, dr[1]);
  u8g.drawStr( 100, 27, dr[2]);
  u8g.drawStr( 100, 45, dr[3]);
  u8g.drawStr( 100, 62, dr[4]);  

}
//=================================================
void blinkLed(int led,int number, int onTime)
//================================================= 
{
  int i;
  for(i=0;i<number;i++)
  {
    digitalWrite(led,HIGH);
    delay(onTime);
    digitalWrite(led,LOW);
    delay(onTime);
  }
}
//=================================================
void NB_serialFlush()
//=================================================
{
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}   

//=================================================
int NB_sendToGwy(int mid, int sid, float data, int other)
//=================================================
{
  int ixSid = 0,i,negative=0;
  char msg1[100],msg2[50],checksum[20];
     strcpy(msg1," ");
     strcpy(msg2," ");
     digitalWrite(yellowLed,HIGH);
     // Mandatory part of message
     sprintf(msg1,"GET /sxndata/index.php?sw=%d&mid=%d&nsid=%d&sid1=%d",app_id,mid,1,sid);
if(g_debug==1){Serial.print("data:");Serial.println(data);}      
     if(mid == NABTON_DATA)
     {
       negative = 0;
       if(data < 0.0)
       {
          negative = 1;
          data = data*(-1.0);
       }
       // Get non-decimal part
       int part1 = floor(data);
if(g_debug==1){Serial.print("part1:");Serial.println(part1);}       
       // Get decimalpart
       float ftemp = (data - part1);
       for(i=1;i<=NFLOAT;i++)ftemp=ftemp*10;
if(g_debug==1){Serial.print("ftemp:");Serial.println(ftemp);}   
       int part2 = round(ftemp);
if(g_debug==1){Serial.print("part2:");Serial.println(part2);}          
       // if negative
       if(negative == 0)
       {
         if(part2 < 10)
           sprintf(msg2,"&name=%s&ip=%s&dat1=%d.0%d",g_clientName,g_sIp,part1,part2);
         else 
           sprintf(msg2,"&name=%s&ip=%s&dat1=%d.%d",g_clientName,g_sIp,part1,part2);
       }
       if(negative == 1)
       {
         if(part2 < 10)
           sprintf(msg2,"&name=%s&ip=%s&dat1=-%d.0%d",g_clientName,g_sIp,part1,part2);
         else 
           sprintf(msg2,"&name=%s&ip=%s&dat1=-%d.%d",g_clientName,g_sIp,part1,part2);
       }
       strcat(msg1,msg2);
     }


    sprintf(dr[4],"S-");
    client.stop();
    if(client.connect(g_server, 80))
     {
       digitalWrite(redLed,HIGH);
       sprintf(dr[4],"S+");
       if(g_debug==1){Serial.print("msg1=");Serial.println(msg1);}
       client.println(msg1);
       //client.println("Host: config.nabton.com");
       client.println("Connection: close");
       client.println();
     }

     digitalWrite(yellowLed,LOW);
     return(other);
}
//=================================================
void clearOled()
//================================================= 
{
  int i;
  for(i=1;i<=4;i++)
  {
    strcpy(dl[i]," ");
    strcpy(dm[i]," ");
    strcpy(dr[i]," ");
  }
}

//=================================================
void setup()
//================================================= 
{
  int i;
  float tempC;  
  String str;
  char c,stemp[40],msg[100],swork[120];

 
  // disable SD SPI
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Serial.begin(9600);
  //NB_serialFlush();

  pinMode(greenLed,          OUTPUT);
  pinMode(redLed,            OUTPUT);
  pinMode(yellowLed,         OUTPUT);

  digitalWrite(greenLed,  HIGH); //Device ON
  digitalWrite(redLed,    HIGH); //No Server
  digitalWrite(yellowLed, HIGH); //No Network

  // OLED
//=================================================

  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  clearOled();
// One Wire
//=================================================

  digitalWrite(greenLed, HIGH); //Device ON
  sprintf(dl[1],"%s",g_clientName);
  sprintf(dr[2],"n");
  sprintf(dr[3],"c");
  sprintf(dr[4],"s");
  sensors.begin();
  nsensors = sensors.getDeviceCount();
  sprintf(dr[1],"%2d",nsensors);
  NB_oledDraw();

//  if(nsensors > 0)
//  {
//    for(i=0;i<nsensors;i++)
//    {
//      sensors.getAddress(device[i], i);
//      sensors.setResolution(device[i], TEMPERATURE_PRECISION);
//    }
//  }

//  sensors.requestTemperatures();
//  for(i=1;i<=nsensors;i++)
//  {
//      tempC = sensors.getTempC(device[i-1]);    
//      str = String(tempC);
//      str.toCharArray(dl[i+1],8); 
//  }
//
//  sprintf(dr[1],"%d",g_device_delay);
//  for(i=1;i<=NSID;i++)
//  {
//    sprintf(dr[i+1],"%d",g_sids[i]);
//  }
// NB_oledDraw();
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
    sprintf(dr[2],"N-");
  }
  else
  {
   digitalWrite(yellowLed, LOW); //Network
   sprintf(dr[2],"N+");
   g_device_state = S_NETWORK;
  }
    
  Serial.print("Local address: ");
  ipAddress = Ethernet.localIP();
  sprintf(g_sIp,"%d.%d.%d.%d", ipAddress[0],ipAddress[1],ipAddress[2],ipAddress[3]);
  Serial.println(ipAddress);
  //sprintf(dm[1],"?");
  //digitalWrite(LED_DEVICE_STATUS,HIGH);
  sprintf(dl[2],"%s",g_sIp);

  NB_oledDraw();
  delay(1000);


//while (g_device_state != S_CONFIGURED)
if(g_device_state == S_NETWORK)
{
// ====== Get global configuration data ========

 
     Serial.print("URL for config: ");Serial.println(g_confServer);
     int res = client.connect(g_confServer, 80);
     Serial.print("URL connect: ");Serial.println(res);
    if(res == 1)
     {
       sprintf(swork,"GET http://%s", g_confServer);
       client.println(swork);
       sprintf(swork,"Host: %s", g_confServer);
       client.println(swork);
       //client.println("Host: config.nabton.com");
       client.println("Connection: close");
       client.println();
       //digitalWrite(LED_INTERNET,LOW);  
       sprintf(dl[3],"GET request...");    
     }
      NB_oledDraw();
      delay(2000);  
      
      int nbytes = client.available();
      if(nbytes > 0)sprintf(dr[3],"C.");
      //Serial.print(nbytes); 
      int count = 0;
      
      for(i=0;i<nbytes;i++) 
      {
         c = client.read();
         //Serial.print(c);
         g_rbuf[count] = c;
         count++;
      }
      g_rbuf[count] = '\0';
      Serial.print("-"); 
      Serial.print(g_rbuf);
      Serial.println("*"); 
      if(strstr(g_rbuf,"SERCON") != NULL)
      {
        sscanf(g_rbuf,"%s %s %d",stemp,g_server,&g_device_delay,g_sids[1]);
        sprintf(dl[3],"%s %d",g_server,g_device_delay);
        sprintf(dr[3],"C+");
        g_device_state = S_CONFIGURED;
      }
      else
       sprintf(dr[3],"C-");

      client.stop();
  
      strcpy(g_rbuf," ");

// ====== End global configuration data ========
} // end while
  sprintf(dm[1],"%3d",g_device_delay);
  NB_oledDraw();
  delay(1000);
  
}
//=================================================
void loop()
//=================================================
{
    int i,j,nbytes,count,itemp,x;
    float tempC;
    char c='1';
    char *p;
    char *str;
    char stemp1[40],stemp2[40],stemp3[40];
    String S_buf;
    sprintf(dm[1],"%3d",g_device_delay);
    sprintf(dl[3],"%s",g_server);
    //sprintf(dl[3],"delay %d",g_device_delay);
    strcpy(g_rbuf," ");
    sensors.requestTemperatures();
    for(j=0;j<nsensors;j++)
    {
  
      sprintf(dl[4],"%3d",g_sids[j+1]);
      tempC = sensors.getTempCByIndex(j);
      dtostrf(tempC,5, NFLOAT, dm[4]);
      if(g_device_state == S_CONFIGURED && tempC != -127)j= NB_sendToGwy(NABTON_DATA,g_sids[j+1],tempC,j);   
      delay(2000); // wait for HTTP response 
      
      //Check for any message from mailbox in server 
      nbytes = client.available();
      //count = 0;
      //S_buf[count] = '\0';
      if(nbytes > 0)
      {
            digitalWrite(redLed,LOW);
            for(i=0;i<nbytes;i++) 
            {
               c = client.read();
               //Serial.print(c);
               S_buf += String(c);
               //count++;
            }
      }
      S_buf.trim();
      //S_buf[count] = '\0';
      //Serial.println(S_buf.length());
      //Serial.print("-"); 
      //Serial.print(S_buf);
      //Serial.println("*"); 
      if(S_buf.indexOf("DATA") != -1) blinkLed(yellowLed,1,50);
      if(S_buf.indexOf("DELAY") != -1)
      {
        x = S_buf.indexOf("DELAY");x = x+6;
        String Stemp = S_buf.substring(x);
        //Serial.println(Stemp);
        g_device_delay = S_buf.substring(x).toInt();
        blinkLed(greenLed,3,100);
        digitalWrite(greenLed,HIGH);
      }

      if(S_buf.indexOf("CONFIG") != -1)
      {
        blinkLed(greenLed,5,200);
        software_Reset();
      }

      S_buf.substring(15,30).toCharArray(stemp1,15);
      sprintf(dl[3],"%s",stemp1);
   
      NB_oledDraw();
      delay(g_device_delay*1000);  
    }
}

