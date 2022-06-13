#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

String MSG = "";
String PHONE = "";
String notif;

bool led_state = true;
bool state = false;

void flash_led(int, int);

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
  } while (notif.indexOf("ok") < 0);

  Serial.println("AT Ready");

  notif = "";

  do
  {
    mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
    updateSerial();

    Serial.println(notif.indexOf("ok") >= 0);
  } while (notif.indexOf("ok") < 0);

  notif = "";
  do
  {
    mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
    updateSerial();
  } while (notif.indexOf("ok") < 0);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(8, OUTPUT);

  flash_led(250, 30);
  digitalWrite(8, LOW);
}

void flash_led(int de, int x)
{
  for (int i = 0; i < x; i++)
  {
    digitalWrite(LED_BUILTIN, led_state);
    led_state = !led_state;
    delay(de);
  }

  digitalWrite(LED_BUILTIN, LOW);
}

void Send_SMS(String msg)
{
  mySerial.println("AT"); // Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CMGS=\"" + PHONE + "\"");
  updateSerial();
  mySerial.print(msg); // text content
  updateSerial();
  mySerial.write(26);
}

void loop()
{
  updateSerial();
  if (notif.startsWith("+cmt"))
  {
    if (notif.length() > 0)
    {
      int idx = notif.indexOf("+cmt: \"");
      PHONE = notif.substring(idx + 7, idx + 20);
      MSG = notif.substring(notif.indexOf('\n') + 1);

      Serial.println(MSG);
      Serial.println(PHONE);
      notif = String();
    }

    if (MSG.equalsIgnoreCase("ON"))
    {
      if (state)
      {
        Send_SMS("Already on.");
      }
      else
      {
        digitalWrite(8, HIGH);
        state = true;
        Send_SMS("ON.");
      }
    }
    else if (MSG.equalsIgnoreCase("OFF"))
    {
      if (state)
      {
        digitalWrite(8, LOW);
        state = false;
        Send_SMS("OFF.");
      }
      else
      {
        Send_SMS("Already off.");
      }
    }
  }
  MSG = String();
  PHONE = String();
}