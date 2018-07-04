#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "Esp32MQTTClient.h"

#define RST_PIN         16         // Configurable, see typical pin layout above
#define SS_PIN          15         // Configurable, see typical pin layout above

SPIClass hspi;
MFRC522 mfrc522(SS_PIN, RST_PIN, &hspi);  // Create MFRC522 instance

// Please input the SSID and password of WiFi
const char* ssid       = "";
const char* password   = "";

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "";

static bool hasIoTHub = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting connecting WiFi.");
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
  
  hspi.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.println(F("Scan PICC"));
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  mfrc522.PICC_HaltA();

  char mes1[50];
  char mes2[50];
  sprintf(mes1, "%02x%02x%02x%02x%02x%02x%02x", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3], mfrc522.uid.uidByte[4], mfrc522.uid.uidByte[5], mfrc522.uid.uidByte[6]);

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  sprintf(mes2, "%s", mfrc522.PICC_GetTypeName(piccType));

  Serial.println(mes1);
  Serial.println(mes2);

  Serial.println("start sending events.");
  if (hasIoTHub)
  {
    char buff[128];

    snprintf(buff, 128, mes1);
    
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
    }
    else
    {
      Serial.println("Failure...");
    }
  }
}

