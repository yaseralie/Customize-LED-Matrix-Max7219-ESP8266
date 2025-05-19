//Display text in LED matrix Max7219, sent from webserver
//Dont forget to install or add zip library ESPAsyncWebServer:https://github.com/me-no-dev/ESPAsyncWebServer
//LED matrix library MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "SSID";
const char* password = "SSID PASSWORD";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

//Setup for LED Max7219==============================
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN 15
// Hardware SPI connection
MD_Parola LED = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
//====================================================

//configuration=======================================
String txt_default = "Keep a safe distance!";
String msg = txt_default;
bool static_led = false;
//===================================================

//Variables for button===============================
int BUTTON_Stop = 5; //button D1
bool status_button_stop = false;

int BUTTON_R = 4; //button D2
bool status_button_R = false;
//===================================================

// HTML web page to handle 2 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Send Text To LED Matrix ESP</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h1>Display Text LED Matrix</h1>
  <form action="/get">
    Send Customize Text: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    Select Defined Text:<br>
  <input type="radio" name="input2" value="Welcome"> <label for="input2">Welcome</label><br>
  <input type="radio" name="input2" value="Thank You Bro!"> <label for="input2">Thank You Bro!</label><br>
  <input type="radio" name="input2" value="Be careful!"> <label for="input2">Be careful!</label><br>
  <input type="radio" name="input2" value="Sorry, I go first"> <label for="input2">Sorry, I go first</label><br>
  <input type="radio" name="input2" value="No overtaking!"> <label for="input2">No overtaking!</label><br>
  <input type="radio" name="input2" value="Traffic Jam ahead"> <label for="input2">Traffic Jam ahead</label><br>
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  //Start LED
  LED.begin();

  //setup button input===============
  pinMode(BUTTON_Stop, INPUT_PULLUP);
  pinMode(BUTTON_R, INPUT_PULLUP);
  //=================================

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    else {
      inputMessage = "";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    //Display Text
    if (inputMessage != "")
    {
      msg = inputMessage;
    }

    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                  + inputParam + ") with value: " + inputMessage +
                  "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  //check trigger stop=================================
  int buttonValue1 = digitalRead(BUTTON_Stop);
  if (buttonValue1 == LOW )
  {
    if (status_button_stop == false)
    {
      static_led = true;
      msg = "STOP";
      status_button_stop = true;  
    }
  }
  else if (buttonValue1 == HIGH)
  {
    if (status_button_stop == true)
    {
      status_button_stop = false;
      msg = txt_default;
      static_led = false;
    }
  }
  //===================================================
  //check trigger reverse=================================
  int buttonValue2 = digitalRead(BUTTON_R);
  if (buttonValue2 == LOW )
  {
    if (status_button_R == false)
    {
      static_led = true;
      msg = "REVERSE";
      status_button_R = true;  
    }
  }
  else if (buttonValue2 == HIGH)
  {
    if (status_button_R == true)
    {
      status_button_R = false;
      msg = txt_default;
      static_led = false;
    }
  }
  //===================================================
 
  //show LED matrix====================================
  if (static_led == false)
  {
    if (LED.displayAnimate())
    LED.displayText(msg.c_str(), PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
  else
  {
    LED.setTextAlignment(PA_CENTER);
    LED.print(msg.c_str());
    delay(500);
    LED.displayClear();
    delay(500);
  }
  //===================================================
  
}
