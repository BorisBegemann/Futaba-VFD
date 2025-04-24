#include <chrono>
#include <thread>
#include <FVFD.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>

int x_counter = 0;
int doScroll = 0;
int scrollSpeed = 1;
int* scrollPtr = &doScroll;
int* scrollSpeedPtr = &scrollSpeed;

AsyncWebServer server(80);
FVFD display;

WiFiUDP udp;
const int UDP_PORT = 31337;
#define DISP_WIDTH 336
#define DISP_HEIGHT 24
const int BUFFER_SIZE = DISP_HEIGHT * DISP_WIDTH / 8;
uint8_t packetBuffer[BUFFER_SIZE];

void setup() {
  WiFiManager wm;
  Serial.begin(115200);
  Serial.println("Booting");
  wm.setHostname("vfd");
  bool res = wm.autoConnect("VFD AP");
  if(!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("connected...yeey :)");
  }

  display.begin();
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", "<html><body><h1>VFD API</h1><h2>Endpoint \"/write\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>size</td><td>1</td></tr><tr><td>x</td><td>0</td></tr><tr><td>y</td><td>0</td></tr><tr><td>scroll</td><td>0</td></tr><tr><td>scrollspeed</td><td>1</td></tr><tr><td>text</td><td>-</td></tr></table><h2>Endpoint \"/setpixel\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>x</td><td>-</td></tr><tr><td>y</td><td>-</td></tr></table><h2>Endpoint \"/clearpixel\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>x</td><td>-</td></tr><tr><td>y</td><td>-</td></tr></table><h2>Endpoint \"/clearscreen\"</h2><h2>UDP</h2><p>You can also send UDP messages to port 31337 with 24*336 bits (1008 bytes) to set the image pixels.</p></body></html>");
  });

  server.on("/write", HTTP_GET, [] (AsyncWebServerRequest *request) {
        int textsize = 1;
        if (request->hasParam("size")) {
          textsize = request->getParam("size")->value().toInt();
        }
        display.setTextSize(textsize);

        int x = 0, y = 0;
        if (request->hasParam("x")) {
          x = request->getParam("x")->value().toInt();
        }
        if (request->hasParam("y")) {
          y = request->getParam("y")->value().toInt();
        }
        display.setCursor(x,y);

        if (request->hasParam("scroll")) {
          if (request->getParam("scroll")->value().toInt()==1)
          {
            *scrollPtr = 1;
          }
          else{
            *scrollPtr = 0;
          }
        }
        else{
          *scrollPtr = 0;
        }

        if (request->hasParam("scrollspeed")) {
          *scrollSpeedPtr = request->getParam("scrollspeed")->value().toInt();
        }
        else{
          *scrollSpeedPtr = 1;
        }

        String message;
        if (request->hasParam("text")) {
          message = request->getParam("text")->value();
          display.clearDisplay();
          display.setTextColor(FVFD_WHITE);
          display.println(message);
        }

        display.display();
        request->send(200, "text/plain");
    });

  server.on("/setPixel", HTTP_GET, [] (AsyncWebServerRequest *request) {
        int x = 0, y = 0;
        *scrollPtr = 0;
        if (request->hasParam("x") && request->hasParam("y")) {
          x = request->getParam("x")->value().toInt();
          y = request->getParam("y")->value().toInt();
          display.drawPixel(x,y,FVFD_WHITE);
          display.display();
        }

        request->send(200, "text/plain");
    });

  server.on("/clearPixel", HTTP_GET, [] (AsyncWebServerRequest *request) {
        int x = 0, y = 0;
        *scrollPtr = 0;
        if (request->hasParam("x") && request->hasParam("y")) {
          x = request->getParam("x")->value().toInt();
          y = request->getParam("y")->value().toInt();
          display.drawPixel(x,y,FVFD_BLACK);
          display.display();
        }

        request->send(200, "text/plain");
    });

    server.on("/clearScreen", HTTP_GET, [] (AsyncWebServerRequest *request) {
      *scrollPtr = 0;
      display.clearDisplay();
      display.display();
      request->send(200, "text/plain");
    });

  server.begin();

  udp.begin(UDP_PORT);
  Serial.printf("UDP server started on port %d", UDP_PORT);
}

void loop() {

  int packetSize = udp.parsePacket();
  if (packetSize) {
    // NOTE: if below line is never reached, maybe you're sending packages that
    // are too large!
    // Serial.println("Packet received");

    if (packetSize != BUFFER_SIZE) {
      byte tempBuffer[packetSize];
      udp.read(tempBuffer, packetSize);
      Serial.printf("Received UDP packet with incorrect size: %d (expected %d)\n", packetSize, BUFFER_SIZE);
      return;
    }

    udp.read(packetBuffer, BUFFER_SIZE);

    *scrollPtr = 0;
    if (x_counter) {
      display.setDisplayOrigin(0, 0);
      x_counter = 0;
    }
    display.drawBitmap(0, 0, packetBuffer, DISP_WIDTH, DISP_HEIGHT, FVFD_WHITE, FVFD_BLACK);
    display.display();
  }

  if (doScroll == 1)
  {
    x_counter = (x_counter + scrollSpeed) % (display.width() - 1);
    display.setDisplayOrigin(x_counter, 0);
  }
  else
  {
    x_counter = 0;
  }
}