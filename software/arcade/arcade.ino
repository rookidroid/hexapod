/*

      This sketch is for the WiFi joystick

      ----------
      Copyright (C) 2022 - PRESENT  Zhengyu Peng
      Website: https://zpeng.me

      `                      `
      -:.                  -#:
      -//:.              -###:
      -////:.          -#####:
      -/:.://:.      -###++##:
      ..   `://:-  -###+. :##:
             `:/+####+.   :##:
      .::::::::/+###.     :##:
      .////-----+##:    `:###:
       `-//:.   :##:  `:###/.
         `-//:. :##:`:###/.
           `-//:+######/.
             `-/+####/.
               `+##+.
                :##:
                :##:
                :##:
                :##:
                :##:
                 .+:

*/

#include <WiFi.h>
#include <WiFiUdp.h>

// GPIO pin number for the LEDs
#define LED_POWER 17
#define LED_CONNECTED 5

#define LED_1 32
#define LED_2 33
#define LED_3 25
#define LED_4 18

// GPIO pin number for the joystick
#define JS_UP 19
#define JS_DOWN 21
#define JS_LEFT 16
#define JS_RIGHT 4

#define BT_UP 27
#define BT_DOWN 12
#define BT_LEFT 26
#define BT_RIGHT 13
#define BT_SPECIAL 14

// PWM channels
#define PWM_POWER 0
#define PWM_CONNECTED 1

// WiFi parameters
const char *ssid = "hexapod";
const char *password = "hexapod_1234";
boolean connected = false;

// UDP
WiFiUDP udp;
const char *udpAddress = "192.168.4.1";
const int udpPort = 1234;

// PWM properties
const int pwmFreq = 5000;
const int pwmResolution = 8;

int js_up = 1;
int js_down = 1;
int js_left = 1;
int js_right = 1;

int bt_up = 1;
int bt_down = 1;
int bt_left = 1;
int bt_right = 1;
int bt_special = 1;

void setup()
{
  // initilize hardware serial:
  Serial.begin(115200);

  pinMode(JS_UP, INPUT_PULLUP);
  pinMode(JS_DOWN, INPUT_PULLUP);
  pinMode(JS_LEFT, INPUT_PULLUP);
  pinMode(JS_RIGHT, INPUT_PULLUP);
  pinMode(BT_UP, INPUT_PULLUP);
  pinMode(BT_DOWN, INPUT_PULLUP);
  pinMode(BT_LEFT, INPUT_PULLUP);
  pinMode(BT_RIGHT, INPUT_PULLUP);
  pinMode(BT_SPECIAL, INPUT_PULLUP);

  // configure LED PWM functionalitites
  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_CONNECTED, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  // blink LEDs
  analogWrite(LED_POWER, 2);
  analogWrite(LED_CONNECTED, 2);

  delay(1000);

  connectToWiFi(ssid, password);
}

void loop()
{
  js_up = digitalRead(JS_UP);
  js_down = digitalRead(JS_DOWN);
  js_left = digitalRead(JS_LEFT);
  js_right = digitalRead(JS_RIGHT);

  bt_up = digitalRead(BT_UP);
  bt_down = digitalRead(BT_DOWN);
  bt_left = digitalRead(BT_LEFT);
  bt_right = digitalRead(BT_RIGHT);
  bt_special = digitalRead(BT_SPECIAL);

  if ((js_up + js_down + js_left + js_right) != 4)
  {
    if ((js_up + js_down + js_left + js_right) == 2)
    {
      if (js_up == 0 && js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkl45:");
        udp.endPacket();
      }
      else if (js_up == 0 && js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkr45:");
        udp.endPacket();
      }
      else if (js_down == 0 && js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkl135:");
        udp.endPacket();
      }
      else if (js_down == 0 && js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkr135:");
        udp.endPacket();
      }
    }
    else
    {
      if (js_up == 0)
      {
        if (bt_up == 0)
        {
          udp.beginPacket(udpAddress, udpPort);
          udp.printf(":fastforward:");
          udp.endPacket();
        } else {
          udp.beginPacket(udpAddress, udpPort);
          udp.printf(":walk0:");
          udp.endPacket();
        }
      }
      else if (js_down == 0)
      {
        if (bt_down == 0) {
          udp.beginPacket(udpAddress, udpPort);
          udp.printf(":fastbackward:");
          udp.endPacket();
        } else {
          udp.beginPacket(udpAddress, udpPort);
          udp.printf(":walk180:");
          udp.endPacket();
        }
      }
      else if (js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkl90:");
        udp.endPacket();
      }
      else if (js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walkr90:");
        udp.endPacket();
      }
    }
  }
  else if ((bt_up + bt_down + bt_left + bt_right + bt_special) != 5)
  {
    if (bt_special == 0) {
      if (bt_up == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":rotatex:");
        udp.endPacket();
      } else if (bt_left == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":rotatey:");
        udp.endPacket();
      } else if (bt_right == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":rotatez:");
        udp.endPacket();
      } else if (bt_down == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":twist:");
        udp.endPacket();
      }
    }
    else if (bt_up == 0)
    {
      if (js_up == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":fastforward:");
        udp.endPacket();
      } else {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walk0:");
        udp.endPacket();
      }
    }
    else if (bt_down == 0)
    {
      if (js_down == 0) {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":fastbackward:");
        udp.endPacket();
      } else {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(":walk180:");
        udp.endPacket();
      }
    }
    else if (bt_left == 0)
    {
      udp.beginPacket(udpAddress, udpPort);
      udp.printf(":turnleft:");
      udp.endPacket();
    }
    else if (bt_right == 0)
    {
      udp.beginPacket(udpAddress, udpPort);
      udp.printf(":turnright:");
      udp.endPacket();
    }
  }
  else
  {
    udp.beginPacket(udpAddress, udpPort);
    udp.printf(":standby:");
    udp.endPacket();
  }
  delay(100);
}

void connectToWiFi(const char *ssid, const char *pwd)
{
  analogWrite(LED_POWER, 2);
  analogWrite(LED_CONNECTED, 0);

  // delete old config
  WiFi.disconnect(true);

  // register event handler
  WiFi.onEvent(WiFiEvent);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

// WiFi event handler
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      // when connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      // initializes the UDP state
      // this initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      analogWrite(LED_POWER, 0);
      analogWrite(LED_CONNECTED, 2);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      analogWrite(LED_POWER, 2);
      analogWrite(LED_CONNECTED, 0);
      break;
    default:
      break;
  }
}
