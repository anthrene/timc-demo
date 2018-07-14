// hello_aquestalk.ino - AquesTalk pico for ESP32
#include "driver/i2s.h"
#include "aquestalk.h"
#include "magic07.h"  // SAMPLE PCM DATA from http://www.kurage-kosho.info/
#define GPIO  25
#define  DMA_BUF_COUNT  3 //  number of DMA buffer
#define  DMA_BUF_LEN    32 // lenght of one DMA buffer (one channnel samples)

#define LEN_FRAME 32
uint32_t workbuf[AQ_SIZE_WORKBUF];

//i2s configuration 
const int i2s_num = 0; // i2s port number
i2s_config_t i2s_config_koe = {
//     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM),
//     .sample_rate = 24000,
     .sample_rate = 12000,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0,
     .dma_buf_count = DMA_BUF_COUNT,
     .dma_buf_len = DMA_BUF_LEN,
     .use_apll = 0
};
i2s_config_t i2s_config_magic = {
//     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
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
i2s_pin_config_t i2s_pin_config = {
     .bck_io_num = 26,
     .ws_io_num = 22,
     .data_out_num = 25,
     .data_in_num = I2S_PIN_NO_CHANGE
};

void setup() {
  int iret;
  Serial.begin(115200);

  Serial.println("Initialize AquesTalk");
  iret = CAqTkPicoF_Init(workbuf, LEN_FRAME, "XXX-XXX-XXX");
  if(iret){
    Serial.println("ERR:CAqTkPicoF_Init");
  }

  DAC_Create(i2s_config_magic);
  Serial.println("Play magic");

// PCM data
  int len2 = sizeof(g_MAGIC07)/sizeof(short);
  for(int k=0;k<len2;k++){
    int16_t val =g_MAGIC07[k];
//    PDMOut_write(val);
    DAC_Write(1, &val);

  }
  
  DAC_Release();
  Serial.println("Stop magic");
//  delay(1000);
  

  DAC_Create(i2s_config_koe);
  Serial.println("D/A start");
  
  Play("yattane.");
  Play("kyouhitotume.");
  Play("tugihananiwosuru?");

  DAC_Release();
  Serial.println("D/A stop");
}

void loop() {
}

void Play(const char *koe)
{
  Serial.print("Play:");
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

////////////////////////////////

void DAC_Create(i2s_config_t conf)
{
  AqResample_Reset();

  //i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_driver_install((i2s_port_t)i2s_num, &conf, 0, NULL);
  //i2s_set_pin((i2s_port_t)i2s_num, NULL);
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
      //uint16_t us = ((uint16_t)wav3[k])^0x8000U;  // signed -> unsigned data å†…è”µDA Only
      //sample[0]=sample[1]=us; // mono -> stereo
      sample[0]=sample[1]=wav3[k]; // mono -> stereo
      int iret = i2s_push_sample((i2s_port_t)i2s_num, (const char *)sample, 100);
      if(iret<0) return iret; // -1:ESP_FAIL
      if(iret==0) break;  //  0:TIMEOUT
    }
  }
  return i;
}
