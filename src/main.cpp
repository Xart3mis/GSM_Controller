#include <Arduino.h>
// #include <ArduinoJson.h>
#include <NeoSWSerial.h>

// StaticJsonDocument<JSON_OBJECT_SIZE(50)> doc;

#define API_KEY "pk.1eb53d351789d96db350712fa4983c80"
#define LED_PIN 5

#define ARR_SIZE 5

#define ASCII_0_VALU 48
#define ASCII_9_VALU 57
#define ASCII_A_VALU 65
#define ASCII_F_VALU 70

NeoSWSerial mySerial(10, 11);

String notif;
String MSG = "";
String PHONE = "";
String request_data = "";

bool led_state = true;
bool state = false;

void timed(void (*func)(), unsigned long _timeout);
void flash_led(int, int);
void send_SMS(String);
void updateSerial();
void parse_SMS();
void get_loc();

void (*resetFunc)(void) = 0;

void setup()
{
  Serial.begin(38400);
  mySerial.begin(38400);

  MSG.reserve(16);
  PHONE.reserve(16);
  notif.reserve(256);
  // request_data.reserve(64);

  Serial.println(F("Initializing..."));
  delay(1000);

  notif = "";
  do
  {
    mySerial.println(F("AT")); // Once the handshake test is successful, it will back to OK
    updateSerial();
    flash_led(85, 1);
  } while (notif.indexOf("ok") < 0);

  notif = "";

  do
  {
    mySerial.println(F("AT+CMGF=1")); // Configuring TEXT mode
    updateSerial();
    flash_led(85, 1);
  } while (notif.indexOf("ok") < 0);

  notif = "";
  do
  {
    mySerial.println(F("AT+CNMI=1,2,0,0,0")); // Decides how newly arrived SMS messages should be handled
    updateSerial();
    flash_led(85, 1);
  } while (notif.indexOf("ok") < 0);

  pinMode(LED_PIN, OUTPUT);
  pinMode(8, OUTPUT);

  flash_led(50, 5);
  digitalWrite(8, LOW);
  // get_loc();
}

void loop()
{
  if (millis() >= 3600000UL)
    resetFunc();

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
        send_SMS(F("Already on."));
      }
      else
      {
        digitalWrite(LED_PIN, HIGH);
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
        digitalWrite(LED_PIN, LOW);
        send_SMS("OFF.");
      }
      else
      {
        send_SMS(F("Already off."));
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
  delay(mySerial.getTimeout());
  while (Serial.available())
  {
    mySerial.write(Serial.read());
  }
  while (mySerial.available())
  {
    notif = mySerial.readStringUntil('\n');
    notif.trim();
    notif.toLowerCase();
    Serial.println(notif);
  }

  mySerial.flush();
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
  mySerial.println(F("AT")); // Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CMGS=\"" + PHONE + "\"");
  updateSerial();
  mySerial.print(msg); // text content
  updateSerial();
  mySerial.write(26);
}

unsigned int HexStringToUInt(char const *hexstring)
{
  unsigned int result = 0;
  char const *c = hexstring;
  char thisC;

  while ((thisC = *c) != NULL)
  {
    unsigned int add;
    thisC = toupper(thisC);

    result <<= 4;

    if (thisC >= ASCII_0_VALU && thisC <= ASCII_9_VALU)
      add = thisC - ASCII_0_VALU;
    else if (thisC >= ASCII_A_VALU && thisC <= ASCII_F_VALU)
      add = thisC - ASCII_A_VALU + 10;
    else
    {
      Serial.print("Unrecognised hex character ");
      Serial.println(thisC);
      return 0;
    }

    result += add;
    ++c;
  }

  return result;
}

void get_loc()
{
  // AT+CNETSCAN=1
  // AT+CNETSCAN

  notif = "";
  do
  {
    mySerial.println(F("AT+CNETSCAN=1"));
    updateSerial();
  } while (notif.indexOf("ok") < 0);

  mySerial.flush();

  mySerial.println(F("AT+CNETSCAN"));
  delay(45000);

  int buf_idx = 0;
  char buf[70 * ARR_SIZE] = {'\0'};

  while (mySerial.available())
  {
    char x = tolower(mySerial.read());
    buf[buf_idx++] = x;
    Serial.write(x);
  }

  buf[buf_idx] = '\n';
  Serial.println();

  notif = String(buf);
  notif.trim();
  Serial.println(notif);

  String shit[ARR_SIZE] = {"\0"};

  notif += '\n';

  int t = 0;
  int r = 0;

  for (uint16_t i = 0; i <= notif.length(); i++)
  {
    if (notif[i] == '\n')
    {
      shit[t] = notif.substring(r, i);
      r = (i + 1);
      t++;
    }
  }

  for (int i = 0; i < ARR_SIZE; i++)
    shit[i] = shit[i].substring(shit[i].indexOf(",") + 1);

  for (int i = 0; i < ARR_SIZE; i++)
    Serial.println(shit[i]);

  notif = "";

  // operator:"mobinil",mcc:602,mnc:01,rxlev:47,cellid:6c17,arfcn:60,lac:8184,bsic:12

  int MCCs[ARR_SIZE] = {0};
  int MNCs[ARR_SIZE] = {0};
  int CIDs[ARR_SIZE] = {0};
  int LACs[ARR_SIZE] = {0};

  // mcc:602,mnc:03,rxlev:27,cellid:1f80,arfcn:514,lac:535e,bsic:04
  for (int i = 0; i < ARR_SIZE; i++)
  {
    Serial.println(shit[i]);
    if (shit[i].length() > 32 && shit[i].indexOf("bsic") > 0)
    {
      MCCs[i] = shit[i].substring(shit[i].indexOf(",mcc:") + 5, shit[i].indexOf(",mnc:")).toInt();
      MNCs[i] = shit[i].substring(shit[i].indexOf(",mnc:") + 5, shit[i].indexOf(",rxlev:")).toInt();
      CIDs[i] = HexStringToUInt(shit[i].substring(shit[i].indexOf(",cellid:") + 8, shit[i].indexOf(",arfcn:")).c_str());
      LACs[i] = HexStringToUInt(shit[i].substring(shit[i].indexOf(",lac:") + 5, shit[i].indexOf(",bsic:")).c_str());
    }
  }

  for (int i = 0; i < ARR_SIZE; i++)
  {
    Serial.println(MCCs[i]);
    Serial.println(MNCs[i]);
    Serial.println(CIDs[i]);
    Serial.println(LACs[i]);
  }

  /*{"token": "pk.1eb53d351789d96db350712fa4983c80", "radio": "gsm", "mcc": 602, "mnc": 3, "cells": [
    {"radio": "gsm", "mcc": 602, "mnc": 3, "lac": 21342, "cid": 8063},
    {"radio": "gsm", "mcc": 602, "mnc": 3, "lac": 21342, "cid": 8064},
    {"radio": "gsm", "mcc": 602, "mnc": 3, "lac": 21342, "cid": 8053}], "address": 1}*/

  // request_data = "{\"token\":\"" + String(API_KEY) + "\", \"radio\":\"gsm\", \"mcc\":" + String(MCCs[1]) + ", \"mnc\":" + String(MNCs[1]) + ", \"cells\":[";
  // request_data = "fdeghwhqwdhwahlahdwhdahhwhdaighddiadawksdgdsadbsakhdgidldsakl;dg sdligdslkdsbudliysad,ksbdhisgdlsbdsldg";
  // Serial.println(request_data);
  // JsonObject root = doc.to<JsonObject>();
  // root["token"] = String(API_KEY);
  // root["radio"] = String("gsm");
  // root["mcc"] = MCCs[1];
  // root["mnc"] = MNCs[1];

  // JsonArray array = root.createNestedArray("cells");
  // root["address"] = 1;

  for (int i = 0; i < ARR_SIZE; i++)
  {
    if (MCCs[i] > 0 && MNCs[i] > 0 && CIDs[i] > 0 && LACs[i] > 0)
    {
      // JsonObject obj = array.createNestedObject();
      // obj["radio"] = String("gsm");
      // obj["mcc"] = MCCs[i];
      // obj["mnc"] = MNCs[i];
      // obj["lac"] = LACs[i];
      // obj["cid"] = CIDs[i];
      // request_data = request_data + "{\"radio\": \"gsm\", \"mcc\": " + String(MCCs[i]) + ", \"mnc\": " + String(MNCs[i]) + ", \"lac\": " + String(LACs[i]) + ", \"cid\": " + String(CIDs[i]) + "}";
    }
  }
  // request_data = request_data + "], \"address\": 1}";

  // serializeJsonPretty(root, Serial);
  // Serial.println(request_data);
}

void parse_SMS()
{
  int idx = notif.indexOf("+cmt: \"");
  PHONE = notif.substring(idx + 7, idx + 20);
  MSG = notif.substring(notif.indexOf('\n') + 1);

  Serial.println(MSG);
  Serial.println(PHONE);
  notif = String();
}

void timed(void (*func)(), unsigned long _timeout)
{
  unsigned long _start_time = millis();
  do
    func();
  while (millis() - _start_time > _timeout);
}