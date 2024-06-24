/*
 *
 *    This sketch is for the WiFi joystick
 *
 *    ----------
 *    Copyright (C) 2022 - PRESENT  Zhengyu Peng
 *    Website: https://zpeng.me
 *
 *    `                      `
 *    -:.                  -#:
 *    -//:.              -###:
 *    -////:.          -#####:
 *    -/:.://:.      -###++##:
 *    ..   `://:-  -###+. :##:
 *           `:/+####+.   :##:
 *    .::::::::/+###.     :##:
 *    .////-----+##:    `:###:
 *     `-//:.   :##:  `:###/.
 *       `-//:. :##:`:###/.
 *         `-//:+######/.
 *           `-/+####/.
 *             `+##+.
 *              :##:
 *              :##:
 *              :##:
 *              :##:
 *              :##:
 *               .+:
 *
 */

#include <WiFi.h>
#include <WiFiUdp.h>

// GPIO pin number for the LEDs
#define LED_POWER 17
#define LED_CONNECTED 5

#define LED_1 32
#define LED_2 25
#define LED_3 33
#define LED_4 18

// GPIO pin number for the joystick
#define JS_UP 16
#define JS_DOWN 17
#define JS_LEFT 19
#define JS_RIGHT 18

#define BT_UP 13
#define BT_DOWN 12
#define BT_LEFT 14
#define BT_RIGHT 15

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


  analogWrite(LED_1, 2);
  delay(1000);
  // analogWrite(PIN_CONNECTED, 0);

  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   Serial.print('.');
  //   delay(500);
  // }
  // Serial.print("Connected! IP address: ");
  // Serial.println(WiFi.localIP());

  // analogWrite(PIN_DISCONNECTED, 0);
  // analogWrite(PIN_CONNECTED, 2);

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

  if ((js_up + js_down + js_left + js_right) != 4)
  {
    if ((js_up + js_down + js_left + js_right) == 2)
    {
      if (js_up == 0 && js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-128:R-255:");
        udp.endPacket();
      }
      else if (js_up == 0 && js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R-128:");
        udp.endPacket();
      }
      else if (js_down == 0 && js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L128:R255:");
        udp.endPacket();
      }
      else if (js_down == 0 and js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R128:");
        udp.endPacket();
      }
      else
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L0:R0:");
        udp.endPacket();
      }
    }
    else
    {
      if (js_up == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R-255:");
        udp.endPacket();
      }
      else if (js_down == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R255:");
        udp.endPacket();
      }
      else if (js_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R-255:");
        udp.endPacket();
      }
      else if (js_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R255:");
        udp.endPacket();
      }
    }
  }
  else if ((bt_up + bt_down + bt_left + bt_right) != 4)
  {
    if ((bt_up + bt_down + bt_left + bt_right) == 2)
    {
      if (bt_up == 0 && bt_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-128:R-255:");
        udp.endPacket();
      }
      else if (bt_up == 0 && bt_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R-128:");
        udp.endPacket();
      }
      else if (bt_down == 0 && bt_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L128:R255:");
        udp.endPacket();
      }
      else if (bt_down == 0 && bt_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R128:");
        udp.endPacket();
      }
      else
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L0:R0:");
        udp.endPacket();
      }
    }
    else
    {
      if (bt_up == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R-255:");
        udp.endPacket();
      }
      else if (bt_down == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R255:");
        udp.endPacket();
      }
      else if (bt_left == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L255:R-255:");
        udp.endPacket();
      }
      else if (bt_right == 0)
      {
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("L-255:R255:");
        udp.endPacket();
      }
    }
  }
  else
  {
    udp.beginPacket(udpAddress, udpPort);
    udp.printf("L0:R0:");
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

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  analogWrite(LED_POWER, 0);
  analogWrite(LED_CONNECTED, 2);
}
