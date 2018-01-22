// Gemaakt door arduino legend Jan

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

const char *ssid     = "alleen voor raspberries";
const char *password = "HUeensff";

ESP8266WebServer server(80);

int ledStatus[3] = { };

void OhNeenWeHebbenEen404() { // Wat nou als mensen niet een resource opvragen die we hebben?
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

const char json[] = R"(
    {
        "Led1": %d,
        "Led2": %d,
        "Led3": %d,
    }
)";

void ZijnErLedjesAan() {
    char buffer[sizeof(json)]; // Dit gaat goed, geloof me het gaat goed
    sprintf(buffer, json, ledStatus[0], ledStatus[1], ledStatus[2]);
    server.send(200, "application/json", buffer);
}

void DoeDeLedjesUpdaten() {
    StaticJsonBuffer<1024> EenKiloJson; // een kilo json is genoeg.
    Serial.println("Er is PUT");
    Serial.println(server.args());
    for (int i = 0; i < server.args(); i++) Serial.println(server.argName(i)), Serial.println(server.arg(i));
    JsonObject& root = EenKiloJson.parseObject(server.arg("plain"));
    root.printTo(Serial);
    Serial.println("");
 
    if (!root.success()) { // Wat nou als er geen geldige JSON is?
        Serial.println("Just one lousy dime, baby Why can't you call me sometime? Oh, no, no Why on earth can't you just pick up the phone, yeah? You know I don't like being alone Why? Why must you torture me? Why you gotta torment me so?");
        return;
    }

    ledStatus[0] = (root["Led1"] == "0" ? 0 : 1);
    ledStatus[1] = (root["Led2"] == "0" ? 0 : 1);
    ledStatus[2] = (root["Led3"] == "0" ? 0 : 1);
    digitalWrite(D1, ledStatus[0]);
    digitalWrite(D2, ledStatus[1]);
    digitalWrite(D3, ledStatus[2]);

    ZijnErLedjesAan(); // stuur de status van de ledjes terug
}

void setup() {
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    digitalWrite(D1, 0);
    digitalWrite(D2, 0);
    digitalWrite(D3, 0);
    
    Serial.begin(115200);
    
    WiFi.begin(ssid, password);
    Serial.println("");
  
    // Wacht op wifi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Doe MDNS dingen
    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }
  
    server.on("/leds", HTTP_GET, ZijnErLedjesAan);
    server.on("/leds", HTTP_PUT, DoeDeLedjesUpdaten);
    server.onNotFound(OhNeenWeHebbenEen404);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}
