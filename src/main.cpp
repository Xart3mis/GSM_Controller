#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define API_KEY "pk.1eb53d351789d96db350712fa4983c80"
#define LED_PIN 5

#define ASCII_0_VALU 48
#define ASCII_9_VALU 57
#define ASCII_A_VALU 65
#define ASCII_F_VALU 70

StaticJsonDocument<JSON_OBJECT_SIZE(48)> doc;

SoftwareSerial mySerial(10, 11);

String MSG = "";
String PHONE = "";
String notif;
String strings;

bool led_state = true;
bool state = false;

void timed(void (*func)(), unsigned long _timeout);
void flash_led(int, int);
void send_SMS(String);
void updateSerial();
void parse_SMS();
void get_loc();

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  while (!Serial || !mySerial)
    flash_led(30, 2);

  timed([]{flash_led(30, 1);}, 1000);

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
  get_loc();
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
  delay(Serial.getTimeout() + mySerial.getTimeout());
  while (Serial.available())
  {
    mySerial.write(Serial.read());
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
  mySerial.println(F("AT")); // Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CMGS=\"" + PHONE + "\"");
  updateSerial();
  mySerial.print(msg); // text content
  updateSerial();
  mySerial.write(26);
}


unsigned int HexStringToUInt(char const* hexstring)
{
    unsigned int result = 0;
    char const *c = hexstring;
    char thisC;

    while( (thisC = *c) != NULL )
    {
        unsigned int add;
        thisC = toupper(thisC);

        result <<= 4;

        if( thisC >= ASCII_0_VALU &&  thisC <= ASCII_9_VALU )
            add = thisC - ASCII_0_VALU;
        else if( thisC >= ASCII_A_VALU && thisC <= ASCII_F_VALU)
            add = thisC - ASCII_A_VALU + 10;
        else
        {
            Serial.print("Unrecognised hex character ");
            Serial.println(thisC);
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

  Serial.setTimeout(5000);
  mySerial.setTimeout(15000);

  mySerial.println(F("AT+CNETSCAN"));
  updateSerial();

  Serial.setTimeout(1000);
  mySerial.setTimeout(1000);

  Serial.println();

  String shit[16] = {"\0"};
  int t = 0;
  int r = 0;

  for (uint16_t i = 0; i < notif.length(); i++)
  {
    if (notif.charAt(i) == '\n')
    {
      shit[t] = notif.substring(r, i);
      r = (i + 1);
      t++;
    }
  }
  notif = "";
  delay(350);
  // operator:"mobinil",mcc:602,mnc:01,rxlev:47,cellid:6c17,arfcn:60,lac:8184,bsic:12

  int MCCs[16] = {0};
  int MNCs[16] = {0};
  int CIDs[16] = {0};
  int LACs[16] = {0};

  // int MCCsMNCsCIDsLACs[16*4];

  for (int i = 0; i < 16; i++)
  {
    if (shit[i].length() > 64)
    {
      Serial.println(shit[i]);
      String MCC, MNC, CID, LAC;

      MCC = shit[i].substring(shit[i].indexOf(",mcc:") + 5, shit[i].indexOf(",mnc"));
      MNC = shit[i].substring(shit[i].indexOf(",mnc:") + 5, shit[i].indexOf(",rxlev"));
      CID = shit[i].substring(shit[i].indexOf(",cell") + 8, shit[i].indexOf(",ar"));
      LAC = shit[i].substring(shit[i].indexOf(",lac:") + 5, shit[i].indexOf(",bsic"));

      Serial.print(MCC);
      Serial.print(" ");
      Serial.print(MNC);
      Serial.print(" ");
      Serial.print(HexStringToUInt(CID.c_str()));
      Serial.print(" ");
      Serial.print(HexStringToUInt(LAC.c_str()));
      Serial.println();

    }
  }

  int idx = 0;

  for (int i = 0; i < 16; i++)
  {
    if (MCCs[i] > 0 && MNCs[i] > 0 && CIDs[i] > 0 && LACs[i] > 0)
      idx++;
  }

  JsonObject root = doc.to<JsonObject>();
  root["token"] = API_KEY;
  root["mcc"] = MCCs[0];
  root["mnc"] = MNCs[0];

  JsonArray array = root.createNestedArray("cells");

  for (int i = 0; i < idx; i++)
  {
    JsonObject obj = array.createNestedObject();
    obj["radio"] = "gsm";
    obj["mcc"] = MCCs[i];
  }

  serializeJsonPretty(root, Serial);
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