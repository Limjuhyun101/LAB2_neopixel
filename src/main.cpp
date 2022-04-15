#include <Arduino.h>
#include <IBMIOTF8266.h>
#include <Adafruit_NeoPixel.h>


#define             ledPin 15
#define             ledNum 4

String user_html = ""
// USER CODE EXAMPLE : your custom config variable 
// in meta.XXXXX, XXXXX should match to ArduinoJson index to access
    "<p><input type='text' name='meta.yourVar' placeholder='Your Custom Config'>";
                    ;
// for meta.XXXXX, this var is the C variable to hold the XXXXX

char*               ssid_pfix = (char*)"IOTneo";
unsigned long       lastPublishMillis = - pubInterval;

int                 red = 0;
int                 green = 0;
int                 blue = 0;


Adafruit_NeoPixel   pixels(ledNum, ledPin, NEO_GRB + NEO_KHZ800);

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    data["r"] = red;
    data["g"] = green;
    data["b"] = blue;


    

    serializeJson(root, msgBuffer);
    client.publish(publishTopic, msgBuffer);

}

void handleUserCommand(JsonDocument* root) {
  JsonObject d = (*root)["d"];

  if(d.containsKey("color")) {
    red = d["color"]["r"].as<int>();
    green = d["color"]["g"].as<int>();
    blue = d["color"]["b"].as<int>();
    for(int i = 0; i < ledNum; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
    pixels.show();
  }
    lastPublishMillis = -pubInterval;;
}

void message(char* topic, byte* payload, unsigned int payloadLength) {
    byte2buff(msgBuffer, payload, payloadLength);
    StaticJsonDocument<512> root;
    DeserializationError error = deserializeJson(root, String(msgBuffer));
  
    if (error) {
        Serial.println("handleCommand: payload parse FAILED");
        return;
    }

    handleIOTCommand(topic, &root);
    if (!strcmp(updateTopic, topic)) {
// USER CODE EXAMPLE : meta data update
// If any meta data updated on the Internet, it can be stored to local variable to use for the logic
        pubInterval = cfg["meta"]["pubInterval"];
// USER CODE EXAMPLE
    } else if (!strncmp(commandTopic, topic, 10)) {            // strcmp return 0 if both string matches
        handleUserCommand(&root);
    }
}

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
  
    pixels.begin();
    pixels.clear();

    initDevice();
    // If not configured it'll be configured and rebooted in the initDevice(),
    // If configured, initDevice will set the proper setting to cfg variable

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());
// USER CODE EXAMPLE : meta data to local variable
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? atoi((const char*)meta["pubInterval"]) : 0;
    lastPublishMillis = - pubInterval;
// USER CODE EXAMPLE
    
    set_iot_server();
    client.setCallback(message);
    iot_connect();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
        iot_connect();
    }
// USER CODE EXAMPLE : main loop
//     you can put any main code here, for example, 
//     the continous data acquistion and local intelligence can be placed here
// USER CODE EXAMPLE
    client.loop();
    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
    }
}