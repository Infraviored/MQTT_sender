#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi network credentials
const char* ssid = "Martin Router King";
const char* password = "Routerking!1312";

// MQTT broker settings
const char* mqtt_server = "homeassistant.local";
const int mqtt_port = 1883;
const char* mqtt_username = "mqtt";
const char* mqtt_password = "Dreierpack1!";

const int TRANSMITTER_PIN = D8;
const int IR_PIN = D7;

// RC device settings
const String devices[][5] = {
  {"A", "1361", "1364", "300", "1"},
  {"B", "4433", "4436", "300", "1"},
  {"C", "5201", "5204", "300", "1"},
  {"D", "5393", "5396", "300", "1"},
  {"E", "4195665", "4195668", "300", "1"},
  {"PC", "13637", "13637", "300", "1"}
};

const int numDevices = sizeof(devices) / sizeof(devices[0]);

#endif
