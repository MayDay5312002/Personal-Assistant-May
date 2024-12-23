#include "Audio.h"
#include "CloudSpeechClient.h"
#include <HardwareSerial.h>
#include <cstring>
#include <WiFi.h>
#include <HTTPClient.h>
// #include "network_param.h"
#include "esp_wpa2.h"
#include <ArduinoJson.h>

#define TX2 17
#define RX2 16
#define BTN_PIN 32

const char* url1 = "Insert the URL for the API endpoint that will create the thread";
const char* url2 = "Insert the URL for the API endpoint that will create the message, associate with thread, and runs the AI assistant";




// String question;


char threadID[50];

void setup() {
  WiFi.disconnect(true);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  pinMode(BTN_PIN, INPUT_PULLUP);
  // delay(500);
  // WiFi.begin(ssid, password);
  /////////WPA Enterprise - Begin
  // WiFi.mode(WIFI_STA);
  // esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); // Initialize WPA2 config
  // esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password)); // Set the password
  // esp_wifi_sta_wpa2_ent_enable(&config);
  // WiFi.begin(ssid);
  /////////WPA Enterprise - End
  // Serial.print("WiFi Connecting");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");  
  // }
  // Serial.printf("\n");
  // Serial.println("WiFi Connected");
  wifiSetUp();///new wifi setup
  HTTPClient httpNew;
  httpNew.setTimeout(15000);
  httpNew.begin(url1);
  int respoCode = httpNew.POST(String("{}"));
  if (respoCode == HTTP_CODE_OK || respoCode == HTTP_CODE_MOVED_PERMANENTLY) {
    String httpRes = httpNew.getString();
    StaticJsonDocument<200> docNew;
    deserializeJson(docNew, httpRes);
    strncpy(threadID, docNew["thread_id"], strlen(docNew["thread_id"]));
  }
  Serial.print(threadID);
  Serial2.printf("READY - ThreadID: %s", threadID);

}

void loop() {
  if(digitalRead(BTN_PIN) == LOW){
    Serial.println("\r\nRecord start!\r\n");
    Serial2.println("Record start!");
    Serial.println("Starting to record");
    Audio* audio = new Audio(ICS43434);
    //Audio* audio = new Audio(M5STACKFIRE);
    audio->Record();
    Serial.println("Recording Completed. Now Processing...");
    Serial2.println("Recording Completed. Now Processing...");
    CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);
    String question = cloudSpeechClient->Transcribe(audio);
    delete cloudSpeechClient;
    delete audio;
    Serial.println(question);
    Serial.printf("Audio:%s\n", question.c_str());

    HTTPClient httpAPI;

     // Set timeout for the HTTP connection
    // Specify the POST request target
    httpAPI.begin(url2);
    httpAPI.addHeader("Content-Type", "application/json"); // Set content type
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("NOT CONNECTED TO WIFI");
    }
    char jsonPayload[512];  // Adjust the size based on expected payload size
    snprintf(jsonPayload, sizeof(jsonPayload), "{\"msg\":\"%s\",\"threadID\":\"%s\"}", question.c_str(), threadID);
    // THIS IS OLD:      String("{\"contents\":[{\"parts\":[{\"text\":\"") + question + String("\"}]}]}");
    // THIS IS NEW:      String("{\"contents\":[{\"parts\":[{\"text\":\"") + question + String("\"}]}],\"generationConfig\":{\"maxOutputTokens\":") + (String)maxToken + String("}}");
    Serial.println(jsonPayload);
    // Send the POST request
    httpAPI.setTimeout(30000); // Set timeout to 15 seconds
    int httpResponseCode = httpAPI.POST(jsonPayload);
    // Check the response code

    if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_MOVED_PERMANENTLY) { //Serial2.println("TESTING");}
      String response = httpAPI.getString();
      // JsonDocument doc;//old
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, response);
      if (error) {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        return;
      }
      //deserializeJson(doc, response.c_str());// old
      Serial.println("Response:");
      Serial.println(response);
        String answerBf = doc["message"];
        answerBf.trim();
        String filteredAnswer;
        filteredAnswer.reserve(answerBf.length());
        for (size_t i = 0; i < answerBf.length(); i++) {
          char c = answerBf[i];
          if (isalnum(c) || isspace(c) || c == '\'' || c == ',' || c == '.' || c == "") {
            filteredAnswer += c;
          } 
          else {
            filteredAnswer += ' ';
          }
        }
        answerBf = filteredAnswer;
        const char* answer = answerBf.c_str();
        Serial.println(answer);
        Serial2.printf("Response:%s\n", answer);
    }
    else {
      Serial.print("Response Code: ");
      Serial.println(httpResponseCode);
    }

  }
}

void wifiSetUp(){
  delay(1000);
  Serial2.println("Ready");
  while(true){
    if(Serial2.available()){
      String temp = Serial2.readStringUntil('\n');
      temp = Serial2.readStringUntil('\n');
      Serial.print(temp);
      Serial.println("---------");
      delay(100);
      if(temp.startsWith("WPA-PERSONAL")){//FOR WPA-PERSONAL
        size_t counter = 0;
        String ssid = Serial2.readStringUntil('\n');
        delay(500);
        String pass = Serial2.readStringUntil('\n');
        ssid.trim();
        pass.trim();
        Serial.print(ssid.c_str());
        Serial.println("---------");
        Serial.println(ssid.length());
        Serial.println();
        Serial.print(pass.c_str());
        Serial.println("---------");
        Serial.println(pass.length());
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");  
        }
        Serial.println();
        Serial.print("Connected");
        return;
      }
      if(temp.startsWith("WPA-ENTERPRISE")){//WPA-ENTERPRISE
        size_t counter = 0;
        String ssid = Serial2.readStringUntil('\n');
        delay(500);
        String username = Serial2.readStringUntil('\n');
        delay(500);
        String password = Serial2.readStringUntil('\n');
        ssid.trim();
        username.trim();
        password.trim();
        Serial.print(ssid);
        Serial.println("---------");
        Serial.print(username);
        Serial.println("---------");
        Serial.print(password);
        Serial.println("---------");
        WiFi.mode(WIFI_STA);
        esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); // Initialize WPA2 config
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username.c_str(), username.length()); // Set the username
        esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username.c_str(), username.length()); // Set the username
        esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password.c_str(), password.length()); // Set the password
        esp_wifi_sta_wpa2_ent_enable(&config);
        WiFi.begin(ssid.c_str());
        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");  
        }
        Serial.println();
        Serial.print("Connected");
        return;
      }
    }
    Serial.println("Waiting...");
    delay(500);
  }
}
