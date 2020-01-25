/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include "secrets.h"

#define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)

uint16_t userVar0 = 0, userVar1 = 0;

// See more at: http://www.esp8266.com/viewtopic.php?f=24&t=13183&start=4#sthash.Ktk1U6vc.dpuf
const char * HTTP_RESP = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"; 

const char * HTML_STRING_HEADER = 
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Magic Door</title><style>"
  "body {font-size:40px; text-align:center; font-family:arial;}"
  "a {font-size: 80px;text-decoration:none;background-color:blue;color:white;padding:10px;border-radius:15px;}"
  "a:active{background-color:red;}"
  "</style>"
  "<script>"
  "window.onload = function() {"
  "var h = setInterval(function() {"
  "let el = document.getElementById('count');"
  "if (el.innerHTML > 0) {"
  "el.innerHTML--;"
  "if (el.innerHTML < 2) {"
  "document.getElementById('seconds').innerHTML = 'second';"
  "}"
  "} else {"
  "document.getElementById('status').innerHTML = 'The door is closed';"
  "clearInterval(h);"
  "}"
  "}, 1000);"
  "}"
  "</script>"
  "</head>"
  "<body>";
  
const char * HTML_STRING_OPENED =   
  "The door is open<hr><br>"
  "<a href=\"../gpio/0\">CLOSE</a>";

const char * HTML_STRING_CLOSED =   
  "The door is closed<hr><br>"
  "<a href=\"../gpio/1\">OPEN</a>"; 
  
const char * HTML_STRING_TEMP_OPEN =   
  "<span id='status'>The door will be open for <span id='count'>3</span> more <span id='seconds'>seconds</span></span><hr><br>"
  "<a href='open'>3 more seconds</a>";
 
const char * HTML_STRING_FOOTER = 
  "</body>"
  "</html>";


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  
  
  WiFi.begin(SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  Blynk.config(BLYNK_AUTH);
  Blynk.connect();

  userSetup();
}

void loop() {
  userLoop();
  Blynk.run();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  String s = HTTP_RESP;
  s += HTML_STRING_HEADER;
  if (req.indexOf("/gpio/0") != -1) {
    userVar0 = 0;
    s += HTML_STRING_CLOSED;
  } else if (req.indexOf("/gpio/1") != -1) {
    userVar0 = 1;
    s += HTML_STRING_OPENED;
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  
  client.flush();

  s += HTML_STRING_FOOTER;

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}
