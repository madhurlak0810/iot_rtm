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

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

constexpr uint8_t RST_PIN = D3;       // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;        // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);        // Instance of the class
MFRC522::MIFARE_Key key;

String tag;
String dbpath = "warehouse A/package ids/";
bool store=false;
String store="store";
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
    

        if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
        {
            sendDataPrevMillis = millis();
            // Write a rfid tag to warehouse A/package id
            if (Firebase.RTDB.setString(&fbdo, dbpath+tag, tag))
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
            store=!store;

            // Write a boolean value to "warehouse A/package id/tag"
            if (Firebase.RTDB.setBool(&fbdo,dbpath+tag+store,store)
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
        tag="";
    }
}