#include <Arduino.h>
#include <WiFi.h>
#include <cstring>
#include <string>
#include "Audio.h"
#include "esp_wpa2.h"
#include <SPI.h>
#include <SD.h>

// Define UART2 pins
#define TX2 17  // Default TX2 pin for ESP32
#define RX2 16  // Default RX2 pin for ESP32

#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

#define SD_CS    5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK  18


#define BTN1  32
#define BTN2  33

size_t volumeVal = 21;

Audio audio;

void wifiSetUp();
void volumeUp();
void volumeDown();

void setup() {
  // Start Serial2 with a baud rate
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  Serial.begin(115200); // For debugging
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  while(true){
    if(Serial2.available()){
      String temp = Serial2.readStringUntil('\n');
      if(temp.startsWith("Ready")){
        Serial.println(temp);
        break;
      }
    } 
    Serial.println("Waiting");
    delay(500);
  }


  
  // WiFi.begin(ssid, password);
  // wifiMulti.addAP(ssid, password);
  //////WPA Enterprise
  // WiFi.mode(WIFI_STA);
  // esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password)); // Set the password
  // esp_wifi_sta_wpa2_ent_enable();
  // WiFi.begin(ssid);
  //////WPA Enterprise
  // Serial.print("Connecting to Wi-Fi");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  //   // wifiMulti.run();
  // }
  // Serial.println("\nConnected to Wi-Fi");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);

  // Attempt to initialize the SD card
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed. Check connections and card format.");
    return;
  }
  Serial.println("SD card initialized successfully.");

  //Connecting to WiFi
  wifiSetUp();

  audio.connecttoFS(SD, "/powerOn.mp3");
  while (audio.isRunning()) {
    audio.loop();
  }
  Serial.println("We can begin!!!");
}

void loop() {
  if (Serial2.available()) {
    String receivedData = Serial2.readStringUntil('\n');
    Serial.println("Received: " + receivedData);

    if (receivedData.startsWith("Response:") && WiFi.status() == WL_CONNECTED) {
      // audio.connecttohost("https://github.com/MayDay5312002/SoundsForMay/raw/refs/heads/main/Beep%20Sound.wav");
      // while (audio.isRunning()) {
      //   audio.loop();
      // }
      Serial.println("It worked");
      String response = receivedData.substring(receivedData.indexOf(":") + 1);
      size_t length = response.length();
      const char* answer = response.c_str();
      Serial.printf("Text Response: %s\n", answer);

      // Dynamically allocate buffer for strtok
      char* answerCpy = (char*)malloc(length + 1); // +1 for null terminator
      if (answerCpy == NULL) {
        Serial.println("Memory allocation failed!");
        return; // Handle gracefully
      }
      strcpy(answerCpy, answer); // Safe copy since we allocated exact length + 1

      // Tokenize and build lines
      char* word = strtok(answerCpy, " ");
      String currentLine = "";
      while (word != NULL) {
        if (currentLine.length() + strlen(word) + 1 <= 80) { // +1 for space
          if (!currentLine.isEmpty()) {
            currentLine += " ";
          }
          currentLine += word;
        } else {
          // Line is ready to be processed
          Serial.println(currentLine);
          if (!audio.connecttospeech(currentLine.c_str(), "en")) {
          Serial.println("Audio playback failed. Retrying...");
          delay(100);
          if (!audio.connecttospeech(currentLine.c_str(), "en")) {
            Serial.println("Retry failed. Skipping this line.");
          }
        }
          while (audio.isRunning()) {
            if(digitalRead(BTN1) == LOW){
              volumeDown();
            }
            if(digitalRead(BTN2) == LOW){
              volumeUp();
            }
            audio.loop();
          }
          Serial.printf("Free heap size: %d\n", ESP.getFreeHeap());

          // Start a new line
          currentLine = word;
        }
        word = strtok(NULL, " ");
      }

      // Process the last line if it exists
      if (!currentLine.isEmpty()) {
        Serial.println(currentLine);
        if (!audio.connecttospeech(currentLine.c_str(), "en")) {
          Serial.println("Audio playback failed. Retrying...");
          delay(100);
          if (!audio.connecttospeech(currentLine.c_str(), "en")) {
            Serial.println("Retry failed. Skipping this line.");
          }
        }
        while (audio.isRunning()) {
          if(digitalRead(BTN1) == LOW){
          volumeDown();
          }
          if(digitalRead(BTN2) == LOW){
            volumeUp();
          }
          audio.loop();
        }
        Serial.printf("Free heap size: %d\n", ESP.getFreeHeap());
      }

      // Free the dynamically allocated memory
      free(answerCpy);
    } else {
      if(receivedData.startsWith("READY")){
        audio.connecttospeech("Starting!", "en");
        while (audio.isRunning()) {
          if(digitalRead(BTN1) == LOW){
            volumeDown();
          }
          if(digitalRead(BTN2) == LOW){
            volumeUp();
          }
          audio.loop();
        }
      }
      if(receivedData.startsWith("Record start")){
        audio.connecttoFS(SD, "/beep.mp3");
        while (audio.isRunning()) {
          if(digitalRead(BTN1) == LOW){
            volumeDown();
          }
          if(digitalRead(BTN2) == LOW){
            volumeUp();
          }
          audio.loop();
        }
        // Serial2.println("Ready");
      }
      if(receivedData.startsWith("Recording Completed")){
        audio.connecttoFS(SD, "/beep.mp3");
        while (audio.isRunning()) {
          audio.loop();
          if(digitalRead(BTN1) == LOW){
            volumeDown();
          }
          if(digitalRead(BTN2) == LOW){
            volumeUp();
          }
        }
      }
      Serial.println("Wi-Fi disconnected OR not the response");
    }

    Serial.printf("Free heap size: %d\n", ESP.getFreeHeap());
    receivedData.clear();
  }
  if(digitalRead(BTN1) == LOW){
    volumeDown();
  }
  if(digitalRead(BTN2) == LOW){
    volumeUp();
  }
}


void volumeUp(){
  if(volumeVal <= 21){
      volumeVal += 5;//increase volume
      Serial.println("increase volume");
      audio.setVolume(volumeVal);
      audio.connecttoFS(SD, "/beep.mp3");
      while(audio.isRunning()){
        audio.loop();
      }
  }
}

void volumeDown(){
  if(volumeVal >= 3){
      Serial.println("decrease volume");

      volumeVal -= 5;//decrease volume
      audio.setVolume(volumeVal);
      audio.connecttoFS(SD, "/beep.mp3");
      while(audio.isRunning()){
        audio.loop();
      }
    }
}

void wifiSetUp(){
  File file = SD.open("/network.txt", FILE_READ);
  Serial.println("Is It working");
  String option = file.readStringUntil('\r');

  Serial.println(option);
  Serial2.println();
  if(option.startsWith("WPA-PERSONAL")){
    Serial.println("WPA-PERSONAL WORKED");
    Serial2.println(option);
    String ssid = file.readStringUntil('\r');
    ssid = ssid.substring(ssid.indexOf(":")+1);
    Serial.print(ssid);
    Serial.println("----------");
    Serial2.println(ssid);
    String pass = file.readStringUntil('\r');
    pass = pass.substring(pass.indexOf(":")+1);
    Serial.print(pass);
    Serial.println("----------");
    Serial2.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");  
    }
    Serial.println();
    Serial.println("Connected");
  }
  else if(option.startsWith("WPA-ENTERPRISE")){
    Serial.println("WPA-ENTERPRISE WORKED");
    Serial2.println(option);
    String ssid = file.readStringUntil('\r');
    ssid = ssid.substring(ssid.indexOf(":")+1);
    Serial.print(ssid);
    Serial.println("----------");
    Serial2.println(ssid);
    String username = file.readStringUntil('\r');
    username = username.substring(username.indexOf(":")+1);
    Serial.print(username);
    Serial.println("----------");
    Serial2.println(username);
    String password = file.readStringUntil('\r');
    password = password.substring(password.indexOf(":")+1);
    Serial.print(password);
    Serial.println("----------");
    Serial2.println(password);
    password.trim();
    ssid.trim();
    username.trim();
    WiFi.mode(WIFI_STA);
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username.c_str(), username.length()); // Set the username
    esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username.c_str(), username.length()); // Set the username
    esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password.c_str(), password.length()); // Set the password
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(ssid.c_str());
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.println("Connected");
  }
  else{
    Serial.println("Error");
  }



  // WiFi.mode(WIFI_STA);
  // esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username)); // Set the username
  // esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password)); // Set the password
  // esp_wifi_sta_wpa2_ent_enable();
  // WiFi.begin(ssid);
  // while (WiFi.status() != WL_CONNECTED) {
  //     Serial.print(".");
  //     delay(500);
  // }
  // Serial.println("");
  // Serial.println("Connected");
}


