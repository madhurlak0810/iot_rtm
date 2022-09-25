#include <Arduino.h>
#include <ESP8266WiFi.h.>
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <MFRC522.h>
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "leaves are blue"
#define WIFI_PASSWORD "M1l1nazeoz"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAIu_4XggIzKsVALa4-HpaaBxLVMs6vyEc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://iot-route-management-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseData gdbo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

constexpr uint8_t RST_PIN = D3; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;  // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;

String tag;
String gpath;
String dbpath = "warehouse A/package ids/";
bool store = true;
String status = "/store";
void setup(void)
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void loop()
{
  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial())
  {
    for (byte i = 0; i < 4; i++)
    {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println(tag);
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    if (Firebase.ready() && signupOK )
    {
      
      if(Firebase.RTDB.getBool(&fbdo,String("warehouse A/package ids/"+tag+status))){
        if (fbdo.dataType() == "boolean")
        {
          store = fbdo.boolData();
          store = !store;
          Serial.println("CHANGED DATA");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        
      }
      else{
        Serial.println("FAILED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("REASON: " + fbdo.errorReason());
      }
      // Write a rfid tag to warehouse A/package id
      if (Firebase.RTDB.setString(&fbdo, dbpath + tag, tag))
      {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      
      

      // Write a boolean value to "warehouse A/package id/tag"
      if (Firebase.RTDB.setBool(&fbdo,dbpath+tag+status,store))
      {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    tag = "";
  }
} 
