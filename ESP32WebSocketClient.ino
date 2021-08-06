#include <ArduinoWebsockets.h>
#include <WiFi.h>

const char* ssid = "ssid"; //Enter SSID
const char* password = "password"; //Enter Password
const char* websockets_server_host = "serverip_or_name"; //Enter server adress
const uint16_t websockets_server_port = 80; // Enter server port


const int ledPin = 2; // ESP32 DevKit LED pin
String pin_data = "";
String Message = "";

int pingCount = 0;

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  //Serial.println(message.data());
  Message = message.data();
  Serial.println(Message);
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

WebsocketsClient client;
void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(1000);
  }

  // run callback when messages are received
  client.onMessage(onMessageCallback);

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  // Connect to server
  client.connect(websockets_server_host, websockets_server_port, "/");

  // Send a heartbeat message
  client.send("\"heartbeat\":\"keepalive\"");
}

void loop() {
  client.poll();

  if (pingCount > 9) {
    pingCount = 0;

    //capture the value of digital pin, send it along
    pin_data = String(digitalRead(ledPin));
    Serial.println("PinStatus: " + pin_data);

    if (pin_data == "1") {
      client.send("\"astate\":\"ON\"");
    } else {
      client.send("\"astate\":\"OFF\"");
    }
  }
  else {
    //Serial.println("I'm listening!");
    if (Message == "?") {
      Serial.println("Sending Status");
      if (pin_data == "1") {
        client.send("\"qstate\":\"ON\"");
      } else {
        client.send("\"qstate\":\"OFF\"");
      }
    } else if (Message == "CMD:on") { //if command then execute
      Serial.println("Sending CMD");
      digitalWrite(ledPin, HIGH);
      client.send("\"cstate\":\"ON\"");

    } else if (Message == "CMD:off") { //if command then execute
      Serial.println("Sending CMD");
      digitalWrite(ledPin, LOW);
      client.send("\"cstate\":\"OFF\"");
    }

    // wait for 10 seconds to send acknowledegment
    pingCount += 1;
    Serial.println(pingCount);
    delay(1000);
  }
}
