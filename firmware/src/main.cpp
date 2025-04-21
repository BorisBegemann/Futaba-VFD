
#include <chrono>
#include <thread>
#include <FVFD.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>

int x_counter = 0;
int doScroll = 0;
int scrollSpeed = 1;
int* scrollPtr = &doScroll;
int* scrollSpeedPtr = &scrollSpeed;

AsyncWebServer server(80);
FVFD display;

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
    request->send_P(200, "text/html", "<html><body><h1>VFD API</h1><h2>Endpoint \"/write\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>size</td><td>1</td></tr><tr><td>x</td><td>0</td></tr><tr><td>y</td><td>0</td></tr><tr><td>scroll</td><td>0</td></tr><tr><td>scrollspeed</td><td>1</td></tr><tr><td>text</td><td>-</td></tr></table><h2>Endpoint \"/setpixel\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>x</td><td>-</td></tr><tr><td>y</td><td>-</td></tr></table><h2>Endpoint \"/clearpixel\"</h2><table><tr><th>Param</th><th>default</th></tr><tr><td>x</td><td>-</td></tr><tr><td>y</td><td>-</td></tr></table><h2>Endpoint \"/clearscreen\"</h2></body></html>");
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
}

void loop() {
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