#include <ESP8266WebServer.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define LCDHEIGHT 64
#define LCDWIDTH 128

#define SEALEVELPRESSURE_HPA (1013.25)

#define SCREEN_NONE        0
#define SCREEN_TEMPERATURE 1
#define SCREEN_HUMIDITY    2
#define SCREEN_PRESSURE    3
#define SCREEN_LIGHT       4
#define SCREEN_NETINFO     5
#define SCREENS            6

#define SERVER_PORT 80

Adafruit_BME280 bme;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/* AP MODE CONFIG */
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
const String apssid = "ESP8266WeatherStation";

/* ICONS */
static const unsigned char PROGMEM temperature_bmp[] = {0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xf8,0x7f,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xc0,0xf,0xfc,0xff,0x80,0x7,0xfc,0xff,0x80,0x7,0xfc,0xff,0x80,0x7,0xfc,0xff,0x80,0x7,0xfc,0xff,0x80,0x7,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xf8,0x7f,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc};
static const unsigned char PROGMEM humidity_bmp[]= {0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xf8,0x7f,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xc0,0xf,0xfc,0xff,0x80,0x7,0xfc,0xff,0x0,0x3,0xfc,0xff,0x0,0x3,0xfc,0xfe,0x0,0x1,0xfc,0xfe,0x0,0x1,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfe,0x0,0x1,0xfc,0xfe,0x0,0x1,0xfc,0xff,0x0,0x3,0xfc,0xff,0x80,0x7,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc};
static const unsigned char PROGMEM pressure_bmp[]= {0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xc0,0xf,0xfc,0xff,0x0,0x3,0xfc,0xfc,0x0,0x0,0xfc,0xf8,0x4,0x80,0x7c,0xf0,0x3c,0xf0,0x3c,0xe0,0xfc,0xfc,0x1c,0xf0,0xff,0xfc,0x3c,0xfc,0x7f,0xf8,0xfc,0xfe,0x7f,0xf9,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0x1f,0xfc,0xff,0xfc,0x3f,0xfc,0xff,0xf8,0x3f,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xf0,0x7f,0xfc,0xff,0xf8,0x7f,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc};
static const unsigned char PROGMEM light_bmp[] = {0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xc0,0xf,0xfc,0xff,0x80,0x7,0xfc,0xff,0x0,0x3,0xfc,0xfe,0x0,0x1,0xfc,0xfe,0x0,0x1,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfc,0x0,0x0,0xfc,0xfe,0x0,0x1,0xfc,0xfe,0x0,0x1,0xfc,0xff,0x0,0x3,0xfc,0xff,0x80,0x7,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xc0,0xf,0xfc,0xff,0xe0,0x1f,0xfc,0xff,0xf0,0x3f,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc};

/* Client mode config */
const int MAX_CONNECT = 20;
const int button = 15;
const int maxRefreshCount = 20;
const int refreshCountBlank = 120;
const int maxResetCount = 40;

// READING VARS
uint addr = 0;
struct { 
  char ssid[32] = "";
  char pass[63] = "";
} wifidata;
String ip;
float temperature, humidity, pressure, altitude, light;
sensors_event_t event;

ESP8266WebServer server(SERVER_PORT);              
 
void setup() {
  pinMode(button, INPUT);
  delay(100);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  if(!bme.begin(0x76)) {
    display.println("No BME280 detected");
    display.display();
    while(1);
  } 
  if(!tsl.begin()) {
    display.println("No TSL2561 detected");
    display.display();
    while(1);
  }
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);

  EEPROM.begin(512);
  EEPROM.get(addr,wifidata);

  if ( strcmp(wifidata.ssid,"") == 0 && strcmp(wifidata.pass, "")==0){
      APMode();
  }else{ 
      ClientMode();
  }
}

void APMode() {
  display.println("CONFIG MODE");
  display.print("\nSetting AP...");
  display.display();
  if(!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    display.println("FAIL");
    display.display();
    while(1);
  }
  display.print("OK\n\nStarting AP...");
  display.display();
  if(!WiFi.softAP(apssid)) {
    display.println("FAIL");
    display.display();
    while(1);
  }
  display.println("OK");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connect to SSID:\n\n" + apssid);
  display.print("\nGo to:\n\nhttp://" + ip2Str(WiFi.softAPIP()) + "/");
  display.display();
  server.on("/", handle_Index_AP);
  server.on("/index.html", handle_Index_AP);
  server.onNotFound(handle_NotFound);
  server.begin();
}

void ClientMode() {
  display.println("Connecting to:\n" + String(wifidata.ssid));
  display.display();

  WiFi.begin(wifidata.ssid, wifidata.pass);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    count++;
    display.print(".");
    display.display();
    if (count == MAX_CONNECT) {
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Can't connect\n\nDevice will RESET...");
      display.display();
      strncpy(wifidata.ssid, "",32);
      strncpy(wifidata.pass, "", 63);
      delay(2000);
      do_reset();
    }
  }
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  
  display.println("Connected to " + String(wifidata.ssid)+ "!");
  ip = ip2Str(WiFi.localIP());
  display.println("\nIP: " + ip);
  display.display();

  server.on("/", handle_Index);
  server.on("/index.html", handle_Index);
  server.on("/measurements", handle_JsonMeasurements);
  server.onNotFound(handle_NotFound);

  server.begin();
  display.println("\nHTTP server started");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
}

String ip2Str(IPAddress ip) {
  String s="";
  for (int i=0; i<4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

//STATE VARS
int lastButtonState = LOW;
int currentButtonState = LOW;
int screen = SCREEN_NONE;
int lastMillis = 0;
int currentMillis = 0;
int refreshCount = 0;
int resetCount = 0;

void loop() {
  server.handleClient();
  currentMillis = millis();
  if (lastMillis <= (currentMillis - 250)) {
    loopEvery250();
    lastMillis = millis();
  }
}

void loopEvery250() {
  currentButtonState = digitalRead(button);
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    lastButtonState = HIGH;
    screen++;
    if(screen == SCREENS) {
      screen = 0;
    }
    refreshCount = 0;
    resetCount = 0;
    showScreen(screen, false);
  } else if(currentButtonState == LOW && lastButtonState == HIGH){
    lastButtonState = LOW;
    resetCount = 0;
  } else if(currentButtonState == HIGH && lastButtonState == HIGH){
    resetCount++;
    if (resetCount == maxResetCount) {
      resetCount = 0;
      strncpy(wifidata.ssid, "", 32);
      strncpy(wifidata.pass, "", 63);
      do_reset();
    }
  }
  if(screen != SCREEN_NONE) {
    refreshCount++;
    if (refreshCount % maxRefreshCount == 0) {
      if(refreshCount == refreshCountBlank) {
        screen = SCREEN_NONE;
        showScreen(screen, false);
        refreshCount = 0;
      } else {
        showScreen(screen, true);
      }
    }
  }
}

void showScreen(int screen, boolean refresh) {
  if (!refresh) {
    display.clearDisplay();
    display.display();
  }
  switch(screen) {
    case SCREEN_NONE:
    break;
    case SCREEN_TEMPERATURE:
      temperature = bme.readTemperature();
      if(!refresh) {
        sensorScreen(temperature_bmp, String(temperature, 2), " C");
        fixCelsius();
      } else {
        updateValue(String(temperature, 2));
      }
    break;
    case SCREEN_HUMIDITY:
      humidity = bme.readHumidity();
      if(!refresh) {
        sensorScreen(humidity_bmp, String(humidity, 2), "%");
      } else {
        updateValue(String(humidity, 2));
      }
    break;
    case SCREEN_PRESSURE:
      pressure = bme.readPressure() / 100.0F;
      if(!refresh) {
        sensorScreen(pressure_bmp, String(pressure, 2), "hPa");
      } else {
        updateValue(String(pressure, 2));
      }
    break;
    case SCREEN_LIGHT:
      tsl.getEvent(&event);
      if (event.light) {
        light = event.light;
      } else {
        light = -1;
      }
      if(!refresh) {
        sensorScreen(light_bmp, String(light, 2), "lux");
      } else {
        updateValue(String(light, 2));
      }
    break;
    case SCREEN_NETINFO:
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("SSID: " + String(wifidata.ssid));
      display.println("\nIP: " + ip);
      display.println("\nPort: " + String(SERVER_PORT));
    break;
  }
  display.display();
}

void do_reset() {
  EEPROM.put(addr,wifidata);
  EEPROM.commit();
  delay(1000);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("RESET DONE");
  display.println("\nDisconnect and connect the device");
  display.display();
  while(1);
}

void handle_Index() {
  String html = "<!DOCTYPE html>\n<html>\n";
  html += "  <head>\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html += "    <title>ESP8266 Weather Station</title>\n";
  html += "    <style>\n      html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  html += "      body{margin-top: 50px;}\n      h1{color: #444444;margin: 50px auto 30px;}\n";
  html += "      p{font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  html += "    </style>\n";
  html += "  </head>\n";
  html += "  <body>\n";
  html += "    <div id=\"webpage\">\n";
  html += "      <h1>ESP8266 Weather Station</h1>\n";
  html += "      <p>Temperature: <span id=\"temperature\"></span> &deg;C</p>\n";
  html += "      <p>Humidity: <span id=\"humidity\"></span> %</p>\n";
  html += "      <p>Pressure: <span id=\"pressure\"></span> hPa</p>\n";
  html += "      <p>Altitude: <span id=\"altitude\"></span> m</p>\n";
  html += "      <p>Light: <span id=\"light\"></span> lux</p>\n";
  html += "    </div>\n";
  html += "  </body>\n";
  html += "  <script>\n";
  html += "    const update = async () => { const response = await fetch('measurements'); const data = await response.json(); Object.keys(data).forEach(key => document.getElementById(key).innerHTML = data[key]); };\n";
  html += "    update().then(() => setInterval(update, 5000));\n";
  html += "  </script>\n";
  html += "</html>\n";
  server.send(200, "text/html", html); 
}

void handle_JsonMeasurements() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  tsl.getEvent(&event);
  if (event.light) {
    light = event.light;
  } else {
    light = -1;
  }
  String json = "{";
  json += "\"temperature\":";
  json += temperature;
  json += ",\"humidity\":";
  json += humidity;
  json += ",\"pressure\":";
  json += pressure;
  json += ",\"altitude\":";
  json += altitude;
  json += ",\"light\":";
  json += light;
  json += "}";
  server.send(200, "application/json", json); 
}

void handle_Index_AP() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    Serial.println("SSID: " + server.arg("ssid") + ", PASS: " + server.arg("pass"));
    server.arg("ssid").toCharArray(wifidata.ssid, 32);
    server.arg("pass").toCharArray(wifidata.pass, 32);
    //strncpy(wifidata.ssid, server.arg("ssid"),32);
    //strncpy(wifidata.pass, server.arg("pass"), 63);
    String html = "<!DOCTYPE html>\n<html>\n";
    html += "  <head>\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    html += "    <title>ESP8266 Weather Station Config</title>\n";
    html += "    <style>\n      html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    html += "      body{margin-top: 50px;}\n      h1{color: #444444;margin: 50px auto 30px;}\n";
    html += "      p{color: #444444;margin-bottom: 10px;}\n";
    html += "    </style>\n";
    html += "  </head>\n";
    html += "  <body>\n";
    html += "    <div id=\"webpage\">\n";
    html += "      <h1>ESP8266 Weather Station Config</h1>\n";
    html += "      <p><span>Successfully configured</span></p>\n";
    html += "      <p><span style=\"color:red;\">Disconnect and connect the weather station.</span></p>\n";
    html += "    </div>\n";
    html += "  </body>\n";
    html += "</html>\n";
    server.send(200, "text/html", html);
    delay(3000);
    do_reset();
  } else {
    String html = "<!DOCTYPE html>\n<html>\n";
    html += "  <head>\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    html += "    <title>ESP8266 Weather Station Config</title>\n";
    html += "    <style>\n      html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    html += "      body{margin-top: 50px;}\n      h1{color: #444444;margin: 50px auto 30px;}\n";
    html += "      p{color: #444444;margin-bottom: 10px;}\n";
    html += "    </style>\n";
    html += "  </head>\n";
    html += "  <body>\n";
    html += "    <div id=\"webpage\">\n";
    html += "      <h1>ESP8266 Weather Station Config</h1>\n";
    html += "      <form action=\"/\" method=\"post\">\n";
    html += "      <p>SSID: <input type=\"text\" name=\"ssid\"/></p>\n";
    html += "      <p>PASS: <input type=\"text\" name=\"pass\"/></p>\n";
    html += "      <p><input type=\"submit\" value=\"Send\"/></p>\n";
    html += "      </form>\n";
    html += "    </div>\n";
    html += "  </body>\n";
    html += "</html>\n";
    server.send(200, "text/html", html);
  }
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

//TEXT DISPLAY VARS
int16_t  cx1, cy1;
uint16_t w, h;

void sensorScreen(const unsigned char *bitmap, String value, String unit) {
  display.drawBitmap(0, LCDHEIGHT/2 - 15, bitmap, 30, 30, 1);
  display.setTextSize(2);
  display.getTextBounds(value, 0, 0, &cx1, &cy1, &w, &h);
  display.setTextColor(WHITE);
  display.setCursor(LCDWIDTH/2 - w/2 + 15, LCDHEIGHT/3 - 2*h/3);
  display.print(value);
  display.getTextBounds(unit, 0, 0, &cx1, &cy1, &w, &h);
  display.setCursor(LCDWIDTH/2 - w/2 + 15, 2*LCDHEIGHT/3 - h/3);
  display.print(unit);
}

void updateValue(String value) {
  display.fillRect(30, 0, LCDWIDTH - 30, LCDHEIGHT/2, BLACK);
  display.setTextSize(2);
  display.getTextBounds(value, 0, 0, &cx1, &cy1, &w, &h);
  display.setTextColor(WHITE);
  display.setCursor(LCDWIDTH/2 - w/2 + 15, LCDHEIGHT/3 - 2*h/3);
  display.print(value);
}

void fixCelsius() {
  display.setCursor(LCDWIDTH/2 - w/2 + 15, 2*LCDHEIGHT/3 - h/3 - 9);
  display.print(".");
}
