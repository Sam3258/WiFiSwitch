/*
  Project 	: WiFi-Switch

  Purpose	: This sketch demonstrates how to use AsyncWebServer to implement WiFi-Switch
  	  	  	  Also, this source code is free software and WITHOUT ANY WARRANTY, enjoy!

  Author	: Sam Chen
  Blog 		: https://www.sam4sharing.com
  Youtube	: https://www.youtube.com/channel/UCN7IzeZdi7kl1Egq_a9Tb4g

  History   :
  Date			Author		Ref		Revision
  20200611		Sam			0.01	Initial version
  20200611		Sam			0.02	Add SD1315 display for IP show-up
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Replace with your WiFi SSID & Password
const char* ssid = "Your SSID";
const char* password = "Password";

IPAddress 	ip;
char 	 	printBuf[64];

// Replace with light control pin as your hardware
uint8_t lightCtrlPin = D5;			// original WiFi Switch uses the GPIO13 / D7 pin
bool lightStatus = LOW;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create object SSD1315
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

void setup() {

	Serial.begin(115200);
	WiFi.setSleepMode(WIFI_NONE_SLEEP);

	// Initial the GPIO pinMode and state
	pinMode(lightCtrlPin, OUTPUT);
	digitalWrite(lightCtrlPin, lightStatus);

	// Start SSD1315
	u8g2.begin();

	// Show-up WiFi info
	Serial.println("WiFi is connecting to ");
	Serial.println(ssid);

	// Start WiFi
	WiFi.begin(ssid, password);

	// Check WiFi status
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	// Print local IP address
	Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
//	Serial.println(WiFi.localIP());
	IPAddress ip = WiFi.localIP();
	Serial.println(ip);

	memset(printBuf, 0, sizeof(printBuf));
	sprintf(printBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

	// Define handles
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		lightStatus = LOW;
		request->send(200, "text/html", SendHTML(lightStatus));
	});

	server.on("/lighton", HTTP_GET, [](AsyncWebServerRequest *request) {
		lightStatus = HIGH;
		Serial.println("Light ON");
		request->send(200, "text/html",  SendHTML(true));
	});

	server.on("/lightoff", HTTP_GET, [](AsyncWebServerRequest *request) {
		lightStatus = LOW;
		Serial.println("Light OFF");
		request->send(200, "text/html", SendHTML(false));
	});

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(404, "text/plain", "Not found");
	});

	server.begin();
	Serial.println("Start WebServer");

	u8g2.clearBuffer();                   		// clear the internal memory
	u8g2.setFont(u8g2_font_ncenB14_tr);			// setup font
	u8g2.drawStr(12, 24, "WiFi Switch");
	u8g2.setFont(u8g2_font_ncenB10_tr);
	u8g2.drawStr(24, 56, printBuf);
	u8g2.sendBuffer();                    		// transfer internal memory to the display
}

void loop() {

	if ( lightStatus ) {
		digitalWrite(lightCtrlPin, HIGH);
	}
	else {
		digitalWrite(lightCtrlPin, LOW);
	}
	delay(500);
}

String SendHTML(uint8_t lightState) {

	String ptr = "<!DOCTYPE html> <html>\n";
	ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
	ptr +="<title>WiFi Switch implementation by AsyncWebServer</title>\n";
	ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
	ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
	ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
	ptr +=".button-on {background-color: #1abc9c;}\n";
	ptr +=".button-on:active {background-color: #16a085;}\n";
	ptr +=".button-off {background-color: #34495e;}\n";
	ptr +=".button-off:active {background-color: #2c3e50;}\n";
	ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
	ptr +="</style>\n";
	ptr +="</head>\n";
	ptr +="<body>\n";
	ptr +="<h1>WiFi Switch</h1>\n";
    ptr +="<h3>Implement by Async WebSever</h3>\n";

    if ( lightState )
    	{ptr +="<p>Light Status: ON</p><a class=\"button button-off\" href=\"/lightoff\">OFF</a>\n";}
    else
    	{ptr +="<p>Light Status: OFF</p><a class=\"button button-on\" href=\"/lighton\">ON</a>\n";}

    ptr +="</body>\n";
    ptr +="</html>\n";
    return ptr;
}
