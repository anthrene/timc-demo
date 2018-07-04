#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         16         // Configurable, see typical pin layout above
#define SS_PIN          15         // Configurable, see typical pin layout above

SPIClass hspi;
MFRC522 mfrc522(SS_PIN, RST_PIN, &hspi);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
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
  sprintf(mes1, "Card UID: %02x %02x %02x %02x", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3]);

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  sprintf(mes2, "PICC type: %s", mfrc522.PICC_GetTypeName(piccType));

  Serial.println(mes1);
  Serial.println(mes2);
}

