// TestPDM.ino - PDM test
//  PCM-data -> I2S -> PDM -> GPIO
//  Copyright (c) 2018 AQUEST
//  
#include "PDMout.h"
#include "magic07.h"  // SAMPLE PCM DATA from http://www.kurage-kosho.info/
#define GPIO  25
#define FS  24000

#define PI  3.14159265

void setup() {
	Serial.begin(115200);
	Serial.println("TestPDM");
}

void loop() {
  Serial.println("D/A create");
  PDMOut_create(GPIO);
  delay(1000);
  
  // PLAY
  for(int iLoop=0;iLoop<4;iLoop++){
    Serial.print("Playing...");
    PDMOut_start(FS);
/*
    // sin波出力
    int len = (int)(0.5*FS);  // 0.5sec
    float rad = 1023.0*2*PI/FS; //1023Hz
    for(int k=0;k<len;k++){
      int16_t val = (int16_t)(32767*sin(rad*k));
      PDMOut_write(val);
    }

    PDMOut_clear();
    delay(500);
*/
    // PCM data
    int len2 = sizeof(g_MAGIC07)/sizeof(short);
    for(int k=0;k<len2;k++){
      int16_t val =g_MAGIC07[k];
      PDMOut_write(val);
    }

    Serial.println("Stop");
    PDMOut_stop();
    
    delay(1000);
  }

  // RELEASE
  Serial.println("Release");
  PDMOut_release();
   
  delay(2000);
}
