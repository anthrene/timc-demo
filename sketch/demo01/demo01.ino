#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "Esp32MQTTClient.h"
#include "driver/i2s.h"
#include "aquestalk.h"
#include "magic07.h"  // SAMPLE PCM DATA from http://www.kurage-kosho.info/

#define RST_PIN         16         // Configurable, see typical pin layout above
#define SS_PIN          15         // Configurable, see typical pin layout above
#define GPIO  25
#define  DMA_BUF_COUNT  3 //  number of DMA buffer
#define  DMA_BUF_LEN    32 // lenght of one DMA buffer (one channnel samples)
#define LEN_FRAME 32
uint32_t workbuf[AQ_SIZE_WORKBUF];
int iret;
int count = 0;

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

//i2s configuration 
static const int i2s_num = 0; // i2s port number
static const i2s_config_t i2s_config_koe = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM),
     .sample_rate = 12000,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0,
     .dma_buf_count = DMA_BUF_COUNT,
     .dma_buf_len = DMA_BUF_LEN,
     .use_apll = 0
};
static const i2s_config_t i2s_config_magic = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM),
     .sample_rate = 24000,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0,
     .dma_buf_count = DMA_BUF_COUNT,
     .dma_buf_len = DMA_BUF_LEN,
     .use_apll = 0
};
static const i2s_pin_config_t i2s_pin_config = {
     .bck_io_num = 26,
     .ws_io_num = 22,
     .data_out_num = 25,
     .data_in_num = I2S_PIN_NO_CHANGE
};

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

  Serial.println("Initialize AquesTalk");
  iret = CAqTkPicoF_Init(workbuf, LEN_FRAME, "XXX-XXX-XXX");
  if(iret){
    Serial.println("ERR:CAqTkPicoF_Init");
  }

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
//  char mes2[50];
  sprintf(mes1, "%02x%02x%02x%02x%02x%02x%02x", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3], mfrc522.uid.uidByte[4], mfrc522.uid.uidByte[5], mfrc522.uid.uidByte[6]);

//  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
//  sprintf(mes2, "%s", mfrc522.PICC_GetTypeName(piccType));

  Serial.println(mes1);
//  Serial.println(mes2);

  Serial.println("start sending events.");
  if (hasIoTHub)
  {
    char buff[128];

    snprintf(buff, 128, mes1);
    
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
      Sound(++count);
      if(count == 3){
        count = 0;
      }
    }
    else
    {
      Serial.println("Failure...");
    }
  }
}

void Sound(int count)
{
  if(count <= 3){
    DAC_Create(i2s_config_magic);
    Serial.println("Start magic");

    int len2 = sizeof(g_MAGIC07)/sizeof(short);
    for(int k=0;k<len2;k++){
      int16_t val =g_MAGIC07[k];
      DAC_Write(1, &val);
    }
  
    DAC_Release();
    Serial.println("Stop magic");  

    DAC_Create(i2s_config_koe);
    Serial.println("Start talking");

    switch(count){
      case 1:
        Talk("yattane.");
        Talk("kyouhitotume.");
        Talk("tugihananiwosuru?");
        break;
      case 2:
        Talk("yattane.");
        Talk("kyouhutatume.");
        Talk("atohitotudane.");
        break;
      case 3:
        Talk("yattane.");
        Talk("kyouhazenbuowattayo.");
        Talk("gannbattane.");
        break;
      default:
        break;
    }
    
    DAC_Release();
    Serial.println("Stop talking");
  }
}

void Talk(const char *koe)
{
  Serial.print("Talk:");
  Serial.println(koe);

  int iret = CAqTkPicoF_SetKoe((const uint8_t*)koe, 100, 0xffffU);
  if(iret)  Serial.println("ERR:CAqTkPicoF_SetKoe");

  for(;;){
    int16_t wav[LEN_FRAME];
    uint16_t len;
    iret = CAqTkPicoF_SyntheFrame(wav, &len);
    if(iret) break; // EOD
    
    DAC_Write((int)len, wav);
  }
}

void DAC_Create(i2s_config_t conf)
{
  AqResample_Reset();

  i2s_driver_install((i2s_port_t)i2s_num, &conf, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &i2s_pin_config);
}

void DAC_Release()
{
  i2s_driver_uninstall((i2s_port_t)i2s_num); //stop & destroy i2s driver 
}

// upsampling & write to I2S
int DAC_Write(int len, int16_t *wav)
{
  int i;
  for(i=0;i<len;i++){
    // upsampling x3
    int16_t wav3[3];
    AqResample_Conv(wav[i], wav3);

    // write to I2S DMA buffer
    for(int k=0;k<3; k++){
      uint16_t sample[2];
      sample[0]=sample[1]=wav3[k]; // mono -> stereo
      int iret = i2s_push_sample((i2s_port_t)i2s_num, (const char *)sample, 100);
      if(iret<0) return iret; // -1:ESP_FAIL
      if(iret==0) break;  //  0:TIMEOUT
    }
  }
  return i;
}

