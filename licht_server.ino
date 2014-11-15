// This is a demo of the RBBB running as webserver with the Ether Card
// 2010-05-28 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>

int led_1 = 3;
int led_2 = 5;
int led_3 = 6;
int led_4 = 9;

boolean led_1_on = true;
boolean led_2_on = true;
boolean led_3_on = true;
boolean led_4_on = true;

boolean strobo = false;
int strobo_an = 100;
int strobo_aus = 100;

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 
  0x00,0x00,0x00,0x00,0x00,0x01 };
static byte myip[] = { 
  192,168,1,10 };

byte Ethernet::buffer[500];
BufferFiller bfill;

void setup () {
  Serial.begin(9600);
  Serial.println("start");
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  //ether.staticSetup(myip);

  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(led_3, OUTPUT);
  pinMode(led_4, OUTPUT);
  
  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
  
  ether.printIp("My IP: ", ether.myip);
}

static word homePage(int value) {
  setValue(value);
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<title>Licht-Server</title>" 
    "<h1>$D</h1>"),
  value);
  return bfill.position();
}

static word notFound() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 404 Not Found\r\n\r\n"));
  return bfill.position();
}


void loop () {
  if(strobo) doStrobo();
  
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);


  if(pos) //check if valid tcp data is received
  {
    
    char* data = (char *) Ethernet::buffer + pos;
    //Serial.print("data: ");
    //Serial.println(data);

    String tmp = String(data);
    String value = tmp.substring(tmp.indexOf('/')+1,tmp.indexOf('HTTP')-4);

    Serial.print("value: ");
    Serial.println(value);
    if(value=="favicon.ico")
    {
      notFound();
    }
    else if(value.startsWith("s"))
    {
      value = value.substring(value.indexOf('/')+1);
      String an = value.substring(value.indexOf('/')+1);
      Serial.print("an: ");
      Serial.println(an);
      strobo_an = an.toInt();
      
      String aus = value.substring(0,value.indexOf('/'));
      Serial.print("aus: ");
      Serial.println(aus);
      strobo_aus = aus.toInt();
      
      strobo = true;
      led_1_on = true;
      led_2_on = true;
      led_3_on = true;
      led_4_on = true;
      
      ether.httpServerReply(homePage(-1));
    }
    else
    {
      strobo = false;
       
      if(value.indexOf('/')>-1) // wenns noch n schläsch hat, dann led auswerten
      {
        String led = value.substring(value.indexOf('/')+1);
        Serial.print("led: ");
        Serial.println(led);
      
        String val = value.substring(0,value.indexOf('/'));
        Serial.print("val: ");
        Serial.println(val);
        value = val;

        led_1_on = false;
        led_2_on = false;
        led_3_on = false;
        led_4_on = false;
        switch(led.toInt()) {
        case 1:  
          led_1_on = true; 
          break;
        case 2:  
          led_2_on = true; 
          break;
        case 3:  
          led_3_on = true; 
          break;
        case 4:  
          led_4_on = true; 
          break;
        default:
          led_1_on = true;
          led_2_on = true;
          led_3_on = true;
          led_4_on = true;
          break;
        }

      }
      else
      {
        led_1_on = true;
        led_2_on = true;
        led_3_on = true;
        led_4_on = true;
      }

      ether.httpServerReply(homePage(value.toInt())); // send web page data
    }
  }
}

void setValue(int value) {
  if(led_1_on) analogWrite(led_1, value);
  if(led_2_on) analogWrite(led_2, value);
  if(led_3_on) analogWrite(led_3, value);
  if(led_4_on) analogWrite(led_4, value);
}

void doStrobo() {
  setValue(1023);
  delay(strobo_an);
  setValue(0);
  delay(strobo_aus);
}


