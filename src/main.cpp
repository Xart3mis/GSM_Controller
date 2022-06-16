#include <Arduino.h>
#include <SoftwareSerial.h>

#define LED_PIN 5

SoftwareSerial mySerial(10, 11); // RX, TX

String MSG = "";
String PHONE = "";
String notif;

String url = "https://www.googleapis.com/geolocation/v1/geolocate?key=AIzaSyB01bnEMWxF4yRLUNMpGJbJX1IVAKrq0w8";

bool led_state = true;
bool state = false;

void flash_led(int, int);
void send_SMS(String);
void updateSerial();
void parse_SMS();
void get_loc();


void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("Initializing...");
  delay(1000);

  notif = "";
  do
  {
    mySerial.println("AT"); // Once the handshake test is successful, it will back to OK
    updateSerial();
    flash_led(250, 2);
  } while (notif.indexOf("ok") < 0);

  Serial.println("AT Ready");

  notif = "";

  do
  {
    mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
    updateSerial();
    flash_led(250, 2);
  } while (notif.indexOf("ok") < 0);

  notif = "";
  do
  {
    mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
    updateSerial();
    flash_led(250, 2);
  } while (notif.indexOf("ok") < 0);

  pinMode(LED_PIN, OUTPUT);
  pinMode(8, OUTPUT);

  flash_led(250, 30);
  digitalWrite(8, LOW);
}

void loop()
{
  updateSerial();
  if (notif.startsWith("+cmt"))
  {
    if (notif.length() > 0)
    {
      parse_SMS();
    }

    if (MSG.equalsIgnoreCase("ON"))
    {
      if (state)
      {
        send_SMS("Already on.");
      }
      else
      {
        digitalWrite(8, HIGH);
        state = true;
        send_SMS("ON.");
      }
    }
    else if (MSG.equalsIgnoreCase("OFF"))
    {
      if (state)
      {
        digitalWrite(8, LOW);
        state = false;
        send_SMS("OFF.");
      }
      else
      {
        send_SMS("Already off.");
      }
    }

  else if (MSG.equalsIgnoreCase("LOC"))
  {
    get_loc();
  }
  }
  MSG = String();
  PHONE = String();
}


void updateSerial()
{
  delay(50);
  while (Serial.available())
  {
    mySerial.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    notif = mySerial.readString();
    notif.trim();
    notif.toLowerCase();
    Serial.println(notif);
  }
}

void flash_led(int de, int x)
{
  for (int i = 0; i < x; i++)
  {
    digitalWrite(LED_PIN, led_state);
    led_state = !led_state;
    delay(de);
  }

  digitalWrite(LED_PIN, LOW);
}

void send_SMS(String msg)
{
  mySerial.println("AT"); // Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CMGS=\"" + PHONE + "\"");
  updateSerial();
  mySerial.print(msg); // text content
  updateSerial();
  mySerial.write(26);
}

void get_loc(){
  /*
  AT+CGATT=1
  AT+SAPBR=3,1,"CONTYPE","GPRS"
  AT+SAPBR =3,1,"APN","etisalat"
  AT+SAPBR=1,1
  AT+SAPBR=2,1
  AT+CIPGSMLOC=1,1
  AT+CIPGSMLOC=2,1
  AT+SAPBR=0,1
  */
  do{
    mySerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    updateSerial();
  } while (notif.indexOf("ok") < 0);
  notif = "";
  do{
    mySerial.println("AT+SAPBR=3,1,\"APN\",\"etisalat\"");
    updateSerial();
  } while (notif.indexOf("ok") < 0);
  notif = "";

  do{
  mySerial.println("AT+SAPBR=1,1");
  updateSerial();
  } while (notif.indexOf("ok") < 0);
  notif = "";

  do{
  mySerial.println("AT+SAPBR=2,1");
  updateSerial();
  } while (notif.indexOf("ok") < 0);
  notif = "";

  do{
  mySerial.println("AT+CIPGSMLOC=2,1");
  updateSerial();
  } while (notif.indexOf("ok") < 0);

  mySerial.println("AT+CIPGSMLOC=1,1");
  updateSerial();
  notif.trim();
  send_SMS(notif);
}

void parse_SMS() {
  int idx = notif.indexOf("+cmt: \"");
  PHONE = notif.substring(idx + 7, idx + 20);
  MSG = notif.substring(notif.indexOf('\n') + 1);

  Serial.println(MSG);
  Serial.println(PHONE);
  notif = String();
}
