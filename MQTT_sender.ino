#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "config.h"
#include "tv_remote_database.h"

RCSwitch mySwitch = RCSwitch(); // Create an instance of the RCSwitch library
WiFiClient espClient;
PubSubClient client(espClient);
IRsend irsend(IR_PIN);
unsigned long lastPublish = 0;

void setup() {
  Serial.begin(115200);
  mySwitch.enableTransmit(TRANSMITTER_PIN);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Subscribe to all device topics
  for (int i = 0; i < numDevices; i++) {
    char topic[10];
    snprintf(topic, 30, "Sender/RC/%s", devices[i][0].c_str());
    client.subscribe(topic);
    Serial.print("Subscribed to ");
    Serial.println(topic);
  }

  // Subscribe to the IR topic
  client.subscribe("Sender/IR");
  Serial.println("Subscribed to Sender/IR");
}

void loop() {
  if (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      
      // Resubscribe to all device topics
      for (int i = 0; i < numDevices; i++) {
        char topic[10];
        snprintf(topic, 10, "Sender/RC/%s", devices[i][0].c_str());
        client.subscribe(topic);
        Serial.print("Subscribed to ");
        Serial.println(topic);
      }
      
      // Resubscribe to the IR topic
      client.subscribe("Sender/IR");
      Serial.println("Subscribed to Sender/IR");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      return;
    }
  }
  client.loop();
  if (millis() - lastPublish > 5000) {
    client.publish("Sender/state", "online");
    lastPublish = millis();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Find the device based on the topic for RC commands
  blinkLED();
  int deviceIndex = -1;
  for (int i = 0; i < numDevices; i++) {
    std::string deviceTopic = std::string("Sender/RC/") + devices[i][0].c_str();
    if (String(topic) == deviceTopic.c_str()) {
      deviceIndex = i;
      break;
    }
  }

  if (deviceIndex != -1) {
    handleRCCommand(devices[deviceIndex], payload, length);
  } else if (String(topic) == "Sender/IR") {
    handleAndSendIRCommand(payload, length);
  } else {
    Serial.println("Received message for unknown device or topic");
  }
}


void handleAndSendIRCommand(byte* payload, unsigned int length) {
  // Parse the message
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Check if the message exists in the IR_CODES_MAP
  if (IR_CODES_MAP.find(message.c_str()) != IR_CODES_MAP.end()) {
    unsigned long irCode = IR_CODES_MAP.at(message.c_str());
    
    // Send the IR code
    IRsend irsend(IR_PIN);
    irsend.begin();

    irsend.sendSAMSUNG(irCode, 32, 1);
    Serial.print("Sent IR code: ");
    Serial.println(message); // Print the name of the IR code instead of the hex value

  } else {
    Serial.print("Received unknown IR command: ");
    Serial.println(message);
  }
}


void handleRCCommand(const String device[], byte* payload, unsigned int length) {
  // Parse the message
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Send the command
  if (message == "on" || message == "ON") {
    sendRCCode(device, true);
  } else if (message == "off" || message == "OFF") {
    sendRCCode(device, false);
  } else {
    Serial.println("Received unknown message for RC command");
  }
}

void sendRCCode(const String device[], bool on) {
  int protocol = device[4].toInt();
  int pulseLength = device[3].toInt();

  mySwitch.setProtocol(protocol);
  mySwitch.setPulseLength(pulseLength);

  int commandCode = on ? device[1].toInt() : device[2].toInt();
  mySwitch.send(commandCode, 24);

  Serial.print("Sent ");
  Serial.print(on ? "on" : "off");
  Serial.print(" command for ");
  Serial.println(device[0]);
}

void blinkLED() {
  const int ledPin = LED_BUILTIN;
  const int blinkDuration = 50; // blink duration in milliseconds

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  delay(blinkDuration);
  digitalWrite(ledPin, HIGH);
}
