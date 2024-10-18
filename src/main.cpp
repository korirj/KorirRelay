///////// External Libraries///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ESPUI.h>
#include <FS.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <MQTT.h> //https://github.com/256dpi/arduino-mqtt
// #include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager/archive/development.zip
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson/releases/download/v6.8.0-beta/ArduinoJson-v6.8.0-beta.zip
#include <PCF8574.h>     //https://github.com/xreef/PCF8574_library
#define HOSTNAME "KorirRelay"
#define MAX_DEVICES 4
#define PCF8574_ADDRESS 0x3F
#define SDA_PIN 4 // D2 on NodeMCU
#define SCL_PIN 5 // D1 on NodeMCU
#define reserConfigPin D5

const int maxNumOfSSIDs = 20;
String stringOne = String("Relay");
String ssids[maxNumOfSSIDs];

WiFiClient net;
MQTTClient client(512);
Ticker sendStat;
#include <Ticker.h>
PCF8574 pcf8574(PCF8574_ADDRESS, SDA_PIN, SCL_PIN);
int numOfssids;

struct LED_LIGHTS
{
  uint8_t pin;
  int buttonPin;
  bool state = false;
  String name;
  uint16_t identity;

} Light[MAX_DEVICES];

char light_topic_in[100] = "";
char light_topic_out[100] = "";
char mqtt_client_name[100] = HOSTNAME;
const char *charPtr;
uint16_t UIssid;
uint16_t UIpassword;
uint16_t UIsaveButton;
uint16_t UIrestartButton;
uint16_t UIMqttServer;
uint16_t UIusername;
uint16_t UImqpassword;

String wifiSSID;
String wifiPassword;
String mqttHost;
String mqttUsername;
String mqttPassword;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

bool wifiConnected = false;
bool wifiConfigured = false;
bool mqttConfigured = false;

void textCall(Control *sender, int type)
{
  uint16_t textboxID = sender->id; // Use sender->id instead of sender->getId()
  String value = sender->value;

  switch (textboxID)
  {
  case 1:
    Serial.println("SSID changed: " + value);
    break;
  case 2:
    Serial.println("Password changed: " + value);
    break;
  case 3:
    Serial.println("MQTT host changed: " + value);
    break;
  case 4:
    Serial.println("MQTT username changed: " + value);
    break;
  case 5:
    Serial.println("MQTT password changed: " + value);
    break;
  default:
    Serial.println("Unknown textbox ID: " + String(textboxID));
  }
}

void callSelect(Control *sender, int type)
{
  // UI_sw_startTime = millis();
  if (sender->id == UIssid)
  {
    int value = sender->value.toInt();
    Serial.println(value);
    // ssids[value].toCharArray(UI_sw_EEPROM.ssid, UI_sw_length);
  }
}

void scanSSIDs()
{
  Serial.println("Scanning for networks...");
  numOfssids = 0;
  WiFi.mode(WIFI_STA);
  // WiFi.disconnect();
  delay(100);

  int foundSSIDs = WiFi.scanNetworks();
  Serial.println("found " + (String)foundSSIDs + " networks");

  if (foundSSIDs)
  {
    for (int i = 0; i < foundSSIDs; ++i)
    {

      //      if ((i < maxNumOfSSIDs) && (WiFi.SSID(i).length() < WIFI_CRED_LENGTH)) { //   -todo. NOTE: if skipping ssids, ssids[i] won't match anymore!!

      if ((i < maxNumOfSSIDs))
      {
        numOfssids++;
        ssids[numOfssids - 1] = WiFi.SSID(i);

        //      Serial.print(WiFi.RSSI(i));
        //      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      }
    }
  }
}




void saveMqttConfig(const char *host, const char *username, const char *password)
{
  if (SPIFFS.begin())
  {
    File mqttConfigFile = SPIFFS.open("/mqttConfig.txt", "w");
    if (mqttConfigFile)
    {
      mqttConfigFile.println(host);
      mqttConfigFile.println(username);
      mqttConfigFile.println(password);
      mqttConfigFile.close();
    }
  }
}

void saveWifiConfig(const char *ssid, const char *password)
{
  if (SPIFFS.begin())
  {
    File configFile = SPIFFS.open("/config.txt", "w");
    if (configFile)
    {
      configFile.println(ssid);
      configFile.println(password);
      configFile.close();
    }
  }
}

void saveConfig()
{
  int valuse = ESPUI.getControl(UIssid)->value.toInt();

  wifiSSID = ssids[valuse];
  wifiPassword = String(ESPUI.getControl(UIpassword)->value);
  mqttHost = String(ESPUI.getControl(UIMqttServer)->value);
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n");
  Serial.println(mqttHost);
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n");

  mqttUsername = String(ESPUI.getControl(UIusername)->value);
  mqttPassword = String(ESPUI.getControl(UImqpassword)->value);

  Serial.println("WiFi SSID: " + wifiSSID);
  Serial.println("WiFi Password: " + wifiPassword);
  Serial.println("MQTT Host: " + mqttHost);
  Serial.println("MQTT Username: " + mqttUsername);
  Serial.println("MQTT Password: " + mqttPassword);

  saveWifiConfig(wifiSSID.c_str(), wifiPassword.c_str());
  saveMqttConfig(mqttHost.c_str(), mqttUsername.c_str(), mqttPassword.c_str());

  // connectToWiFi();
  // setupRelays();
  // server.send(200, "text/plain", "Configurations saved. Restarting...");
  delay(1000);
  ESP.restart();
}

void buttonCallback(Control *sender, int type)
{
  saveConfig();
  //  ESP.restart();
}

void setupConfigPortal()
{
  // scanSSIDs();

  UIssid = ESPUI.addControl(ControlType::Select, "SSID", "", ControlColor::Peterriver, Control::noParent, &callSelect);
  for (int i = 0; i < numOfssids; i++)
  {
    charPtr = ssids[i].c_str();
    ESPUI.addControl(ControlType::Option, charPtr, (String)i, ControlColor::Peterriver, UIssid);
  }

  // if (numOfssids)   // set ssid to first selection
  // ssids[0].toCharArray(UI_sw_EEPROM.ssid, UI_sw_length);

  // UIssid = ESPUI.text("SSID:", &textCall, ControlColor::Alizarin, "SSID");
  UIpassword = ESPUI.text("Wifi Password", &textCall, ControlColor::Alizarin, "Wifi Password");
  UIMqttServer = ESPUI.text("mqttServer", &textCall, ControlColor::Alizarin, "MQTT Server");
  UIusername = ESPUI.text("MQTT Username", &textCall, ControlColor::Alizarin, "MQTT Username");
  UImqpassword = ESPUI.text("MQTT Password", &textCall, ControlColor::Alizarin, "MQTT Password");
  UIsaveButton = ESPUI.button("Save", &buttonCallback, ControlColor::Peterriver, "Save");

  dnsServer.start(DNS_PORT, "*", apIP);

  Serial.println("\n\nWiFi parameters:");
  Serial.print("Mode: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  Serial.print("IP address: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());

  ESPUI.begin("Configuration");
}

void listAllFiles()
{
  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    String ame = dir.fileName();
    Serial.println("available file: " + ame);
  }
}
void clearSPIFFS(String fileName)
{

  SPIFFS.remove(fileName);
  Serial.println("Removing file: " + fileName);
}

void deleteFile(String path)
{

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
    return;
  }
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }

  LittleFS.end();
}

void resetConfigs()
{

  deleteFile("/mqttConfig.txt");
  deleteFile("/config.txt");

  Serial.println("ResetDone");
  delay(5000);
}

String statusMsg(void)
{
  /*
  Will send out something like this:
  {
    "light1":"OFF",
    "light2":"OFF",
    "light3":"OFF",
    "light4":"OFF",
    "light5":"OFF",
    "light6":"OFF",
    "light7":"OFF",
    "light8":"OFF"
  }
  */

  JsonDocument json;
  for (uint8_t i = 0; i < MAX_DEVICES; i++)
  {
    // String l_name = "light" + String(i + 1);
    json[Light[i].name] = (Light[i].state) ? "ON" : "OFF";
  }
  String msg_str;
  serializeJson(json, msg_str);
  return msg_str;
}



void sendMQTTStatusMsg(void)
{
  Serial.print(F("Sending ["));
  Serial.print(light_topic_out);
  Serial.print(F("] >> "));
  Serial.println(statusMsg());
  client.publish(light_topic_out, statusMsg());
  sendStat.detach();
}

void setLights(uint8_t single_pin = 99)
{
  uint8_t begin_i = 0, end_i = MAX_DEVICES;
  if (single_pin != 99)
  {
    begin_i = single_pin;
    end_i = single_pin + 1;
  }
  for (begin_i; begin_i < end_i; begin_i++)
  {
    pcf8574.digitalWrite(Light[begin_i].pin, Light[begin_i].state); // active LOW is relay ON
    ESPUI.updateSwitcher(Light[begin_i].identity, Light[begin_i].state);
  }
  sendMQTTStatusMsg();
}

void switchCallback(Control *sender, int value)
{

  for (uint8_t i = 0; i < MAX_DEVICES; i++)
  {

    if (Light[i].identity == sender->id)
    {
      uint8_t ident = Light[i].pin;
      Serial.print(" ");
      Serial.println(sender->id);

      switch (value)
      {
      case S_ACTIVE:
        Serial.print("Active:");
        Light[ident].state = true;

        setLights(ident);
        break;

      case S_INACTIVE:
        Serial.print("Inactive");
        Light[ident].state = false;
        setLights(ident);
        break;
      }
    }
  }
}

void initLights(void)
{
  for (uint8_t i = 0; i < MAX_DEVICES; i++)
  {
    Light[i].pin = i;
    Light[i].buttonPin = i + (MAX_DEVICES);
    ;
    Light[i].name = stringOne + i;

    pcf8574.pinMode(Light[i].pin, OUTPUT);
    pcf8574.pinMode(Light[i].buttonPin, INPUT_PULLUP);
  }
  pcf8574.begin();

  ESPUI.begin("Power Control");

  for (int i = 0; i < MAX_DEVICES; i++)
  {
    Light[i].identity = ESPUI.switcher(Light[i].name.c_str(), &switchCallback, ControlColor::Alizarin, Light[i].state);

    // ESPUI.Switcher(relays[i].name, relays[i].state, updateRelayCallback);
  }
}


void processJson(String &payload)
{
  /*
  incoming message template:
  {
    "light": 1,
    "state": "ON"
  }
  */

  JsonDocument jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error)
  {
    Serial.print(F("parseObject() failed: "));
    Serial.println(error.c_str());
  }
  JsonObject root = jsonBuffer.as<JsonObject>();

  if (root[stringOne].is<JsonVariant>())
  {
    uint8_t index = jsonBuffer[stringOne];
    index--;
    if (index >= MAX_DEVICES)
      return;
    String stateValue = jsonBuffer["state"];
    Serial.println(stateValue);
    Serial.println(stateValue.length());

    Serial.println(stateValue);
    if (stateValue == "ON" or stateValue == "on")
    {
      Light[index].state = true;
      setLights(index);
      // sendMQTTStatusMsg();
      // webSocket.broadcastTXT(statusMsg().c_str());
      // writeEEPROM();
    }
    else if (stateValue == "OFF" or stateValue == "off")
    {
      Light[index].state = false;
      setLights(index);
      // sendMQTTStatusMsg();
      // webSocket.broadcastTXT(statusMsg().c_str());
      // writeEEPROM();
    }
  }
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: [" + topic + "] << " + payload);
  processJson(payload);
}

void sendAutoDiscoverySwitch(String index, String &discovery_topic, uint16_t identity)
{
  /*
  "discovery topic" >> "homeassistant/switch/XXXXXXXXXXXXXXXX/config"

  Sending data that looks like this >>
  {
    "name":"lights1",
    "state_topic": "home/aabbccddeeff/out",
    "command_topic": "home/aabbccddeeff/in",
    "payload_on":"{'light':1,'state':'ON'}",
    "payload_off":"{'light':1,'state':'OFF'}",
    "value_template": "{{ value_json.light1.value }}",
    "state_on": "ON",
    "state_off": "OFF",
    "optimistic": false,
    "qos": 0,
    "retain": true
  }
  */

  // const size_t capacity = JSON_OBJECT_SIZE(11) + 600;
  JsonDocument json;

  json["name"] = stringOne + " " + index;
  json["state_topic"] = light_topic_out;
  json["command_topic"] = light_topic_in;
  json["payload_on"] = "{" + stringOne + ":" + index + ",'state':'ON'}";
  json["payload_off"] = "{" + stringOne + ":" + index + ",'state':'OFF'}";
  json["value_template"] = "{{value_json.light" + index + "}}";
  json["state_on"] = "ON";
  json["state_off"] = "OFF";
  json["optimistic"] = false;
  json["qos"] = 0;
  json["retain"] = true;

  String msg_str;
  Serial.print(F("Sending AD MQTT ["));
  Serial.print(discovery_topic);
  Serial.print(F("] >> "));
  serializeJson(json, Serial);
  serializeJson(json, msg_str);
  client.publish(discovery_topic, msg_str, true, 0);
  Serial.println();
}

void sendAutoDiscovery()
{
  for (uint8_t i = 0; i < MAX_DEVICES; i++)
  {
    String dt = "homeassistant/switch/" + String(HOSTNAME) + String(i + 1) + "/" + stringOne + String(i + 1) + "/config";
    sendAutoDiscoverySwitch(String(i + 1), dt, Light[i].identity);
  }
}

void connect_mqtt(void)
{
  Serial.print(F("Checking wifi "));
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println(F(" connected!"));

  uint8_t retries = 0;
  Serial.println(F("Connecting MQTT "));
  Serial.println(mqttHost);
  client.begin(mqttHost.c_str(), net); // Replace with your MQTT broker port
  client.onMessage(messageReceived);
  Serial.println(mqttUsername);
  Serial.println(mqttPassword);
  while (!client.connect(HOSTNAME, mqttUsername.c_str(), mqttPassword.c_str()) and retries < 15)
  {
    Serial.print(".");
    delay(5000);
    retries++;
  }
  if (!client.connected())
    ESP.restart();
  Serial.println(F(" connected!"));

  // we are here only after sucessful MQTT connect
  client.subscribe(light_topic_in);      // subscribe to incoming topic
  sendAutoDiscovery();                   // send auto-discovery topics
  sendStat.attach(2, sendMQTTStatusMsg); // send status of switches
}
void readWifiConfig()
{
  if (SPIFFS.begin())
  {
    File configFile = SPIFFS.open("/config.txt", "r");
    if (configFile)
    {
      String ssid = configFile.readStringUntil('\n');
      ssid.trim();
      Serial.println(ssid);
      String password = configFile.readStringUntil('\n');
      password.trim();
      Serial.println(password);

      configFile.close();
      if (ssid.length() > 0 && password.length() > 0)
      {
        wifiConfigured = true;
      }
    }
  }
}

void updateButtons()
{
  for (int i = 0; i < MAX_DEVICES; i++)
  {

    if (pcf8574.digitalRead(Light[i].buttonPin) == LOW)
    {
      Light[i].state = !Light[i].state;
      setLights(i);
      delay(1000);
    }
    // Assuming LOW when button is pressed
    // ESPUI.updateSwitcher(Light[i].identity, Light[i].state);
    // sendMQTTStatusMsg();
    // (Light[begin_i].state) ? LOW : HIGH)
  }
}

void readMqttConfig()
{
  if (SPIFFS.begin())
  {
    File mqttConfigFile = SPIFFS.open("/mqttConfig.txt", "r");
    if (mqttConfigFile)
    {
      mqttHost = mqttConfigFile.readStringUntil('\n');
      mqttHost.trim();
      Serial.println("Here comes the host: ");

      Serial.println(mqttHost);
      mqttUsername = mqttConfigFile.readStringUntil('\n');
      mqttUsername.trim();
      Serial.println("Here comes the username: ");

      Serial.println(mqttUsername);

      mqttPassword = mqttConfigFile.readStringUntil('\n');
      mqttPassword.trim();
      Serial.println("Here comes the password: ");
      Serial.println(mqttPassword);

      mqttConfigFile.close();
      if (mqttHost.length() > 0 && mqttUsername.length() > 0 && mqttPassword.length() > 0)
      {
        mqttConfigured = true;
      }
    }
  }
}

void createHotspot()
{
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.hostname(HOSTNAME);

  WiFi.softAP(HOSTNAME);
  int timeout = 5;

  do
  {
    delay(500);
    Serial.print(".");
    timeout--;
  } while (timeout);

  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("DNS server started");

  // Serial.println("\n\nWiFi parameters:");
  // Serial.print("Mode: ");
  // Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}

void connectToWiFi()
{
  // readWifiConfig();
  WiFi.hostname(HOSTNAME);
  if (wifiConfigured)
  {
    File configFile = SPIFFS.open("/config.txt", "r");
    String ssid = configFile.readStringUntil('\n');
    // Serial.println(ssid.length());

    Serial.println("i have the ssid:");
    // Serial.println(sssid.length());
    ssid.trim();
    Serial.println(ssid.length());

    String password = configFile.readStringUntil('\n');
    password.trim();
    Serial.println("i have the password:");
    // Serial.println(ppassword);
    // Serial.println(ppassword.length());
    // String password =ppassword.substring(0, ppassword.length()-1);
    Serial.println(password);
    Serial.println(password.length());

    configFile.close();
    WiFi.mode(WIFI_STA);
    delay(1000);
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      wifiConnected = true;
      Serial.println("\nConnected to WiFi");
    }
    else
    {
      Serial.println("\nFailed to connect to WiFi");
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting setup...");
  listAllFiles();
  pinMode(reserConfigPin, INPUT_PULLUP);
  if (!digitalRead(reserConfigPin))
  {
    delay(3000);
    if (!digitalRead(reserConfigPin))
    {
      Serial.println("Reseting configs...");
      resetConfigs();
    }
  }
  //  resetConfigs();

  readWifiConfig();
  readMqttConfig();

  if (!wifiConfigured || !mqttConfigured)
  {
    Serial.println("Wi-Fi or MQTT not configured. Setting up Config Portal...");
    scanSSIDs();
    createHotspot();
    delay(1000);
    setupConfigPortal();
  }
  else
  {
    Serial.println("Connecting to Wi-Fi...");
    connectToWiFi();

    byte mac[6];
    WiFi.macAddress(mac);
    sprintf(mqtt_client_name, "%02X%02X%02X%02X%02X%02X%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], HOSTNAME);
    sprintf(light_topic_in, "home/%02X%02X%02X%02X%02X%02X%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], "/in");
    sprintf(light_topic_out, "home/%02X%02X%02X%02X%02X%02X%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], "/out");

    connect_mqtt();
    // dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("Initializing Lights...");
    initLights();
  }

  Serial.println("Setup complete.");
}

void loop()
{
  dnsServer.processNextRequest();

  if (wifiConnected)
  {
    // Serial.println("Wi-Fi connected. Entering main loop...");
    client.loop();
    updateButtons();
    // Additional debug prints or actions can be added here.
  }
}