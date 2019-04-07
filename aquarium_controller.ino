
#include <ESP8266WiFi.h>
#include <urlencode.h>
#include <string.h>
#include <ArduinoJson.h>
#include "credentials.h"
#include <ESP8266WebServer.h>   // Include the WebServer library

 
const char* host = "192.168.2.101";
const int httpPort = 3001;
String authentication_token = "";
const char* device = "";
const char* json_buffer = "";
String name = "ro";
boolean loggedIn = false;


//settings
boolean on = false;

unsigned long currentMillis = 0;
unsigned long previousReportMillis = 0;

int valvePin = 12;//D6


ESP8266WebServer server(80);
String header;


boolean makeRequest(String endpoint, String params, boolean auth, String type){
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return false;
  }
  // We now create a URI for the request
  String url = "/";

  url += endpoint;

  url += "?device[name]=";
  url += urlencode(name);
  url += params;
  Serial.println(url);
  if (auth) {
    client.print(type + " " + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "AUTHORIZATION: " + authentication_token + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else {
    client.print(type + " " + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return false;
    }
  }
  bool status_ok = false;
  StaticJsonBuffer<700> jsonBuffer;
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line.substring(9, 15) == "200 OK"){ status_ok = true;}
    else if (line.substring(9, 25) == "401 Unauthorized"){
      status_ok = false;
      loggedIn = false;
    }
    JsonObject& root = jsonBuffer.parseObject(line);
    root.printTo(Serial);
    // Test if parsing succeeds.
    if (!root.success()) {

    }
    else{
      if (root.containsKey("authentication_token")){
        String auth_token = root["authentication_token"];
        authentication_token = auth_token;
      }
      if (root.containsKey("settings")){
        JsonObject& settings = root["settings"];
        if (settings.containsKey("on")){
          Serial.println("onnn kurwaaa!!");
          on = settings["on"];
        }
        
      }
      loggedIn = true;
    }
  }
  return status_ok;
}


void reportData(){
  if (millis() >= previousReportMillis + 60000){

    
    String endpoint = "reports";
    String params = "";
    params += "&device[reports][checkin]=true";
    boolean requestSucceeded = makeRequest(endpoint, params,  true, "POST");
    if (requestSucceeded){
      previousReportMillis = millis();
    }
    
   }
}

void logIn(){
  String endpoint = "new_session";
  String params = "";
  params += "&device[password]=";
  params += urlencode(device_password);
  boolean requestSucceeded = false;
  while (requestSucceeded == false){
    requestSucceeded = makeRequest(endpoint, params,  false, "GET");
  }
}


void setValve(){
  if (on == true){
    digitalWrite(valvePin, HIGH);
  }
  else{
    digitalWrite(valvePin, LOW);
  }
}

//     This rutine is exicuted when you open its IP in browser
//===============================================================


void setup() {
  pinMode(valvePin, OUTPUT);

  digitalWrite(valvePin, LOW);
  
  Serial.begin(115200);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.println("dupa");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); // <<< Station
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
   
  Serial.println("");
  Serial.println("WiFi connected");
   
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  logIn();
  
  Serial.println(authentication_token);


  Serial.println();
  Serial.println("closing connection");
}

void loop() {
  if (loggedIn == false){
     logIn();
  }
  

  
  reportData();
  setValve();
  server.handleClient();          //Handle client requests
}




