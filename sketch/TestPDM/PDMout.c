// PDMOut.c - PDM audio output for ESP32
// 2018/03/31 N.Yamazaki(AQUEST)  creation.

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/dac.h>
#include <driver/rtc_io.h>
#include "PDMout.h"

#define  DMA_BUF_COUNT  3 //  number of DMA buffer
#define  DMA_BUF_LEN    32 // lenght of one DMA buffer (one channnel samples)
#define  DMA_BUF_SIZE   (DMA_BUF_COUNT*DMA_BUF_LEN) // total DMA buffer size [sample]
#define  I2S_FIFO_LEN   (64/2)
static const i2s_port_t i2s_num = (i2s_port_t)0; // i2s port number.  I2S_1 has not PDM. never change this.

static int _WriteOne(int16_t val);

static gpio_num_t gGpio=0;
static dac_channel_t  gDacCh = -1;	// gDacChANNEL_2; // GPIO26
static TickType_t gTicsWait;
///////////////////////////////////////////
// PDMOut  Create
// gpio: output pin. only  25 or 26 
void PDMOut_create(int gpio)
{
  if(gpio==25)  gDacCh = DAC_CHANNEL_1;
  else if(gpio==26) gDacCh = DAC_CHANNEL_2;
  else return;
  gGpio = (gpio_num_t)gpio;

  // rise to middle level by DAC
  dac_output_voltage(gDacCh, 0);
  dac_output_enable(gDacCh);
  for(int k=0;k<=128;k++){
    dac_output_voltage(gDacCh, k);
    delayMicroseconds(70);
  }
  // keep middle level by DAC
}

///////////////////////////////////////////
// PDMOut  Relese
void PDMOut_release()
{
  // down sloop
  for(int k=127;k>=0;k--){
    dac_output_voltage(gDacCh, k);
    delayMicroseconds(70);
  }

  // Prepare Digital Gpio as OUT/LOW  
  gpio_pad_select_gpio(gGpio);
  gpio_set_direction(gGpio, GPIO_MODE_OUTPUT);
  gpio_set_level(gGpio, 0);

  //Select Gpio as Digital Gpio
  rtc_gpio_deinit(gGpio); 
 
  // dac disable
  dac_output_disable(gDacCh);
}
//#include <soc/i2s_reg.h>  //I2S_PDM_CONF_REG
///////////////////////////////////////////
// PDMOut  Start I2S+PDM
// freq: sampling rate (ex: 8000, 44100…) [Hz]
void PDMOut_start(int freq)
{
  // init I2S+PDM
  i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM ),
     .sample_rate = freq,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0,
     .dma_buf_count = DMA_BUF_COUNT,
     .dma_buf_len = DMA_BUF_LEN,
     .use_apll = 0
  };
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
 
  i2s_pin_config_t pin_config = {
      //.bck_io_num = I2S_PIN_NO_CHANGE,
      //.ws_io_num = I2S_PIN_NO_CHANGE,
      .bck_io_num = 26,
      .ws_io_num = 22,
      .data_out_num = gGpio,
      .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_set_pin(i2s_num, &pin_config);
    
	// switch to PDM  from DAC
  rtc_gpio_deinit(gGpio);
  dac_output_disable(gDacCh);

  // calc ticks to wait
  gTicsWait = (TickType_t)(2*DMA_BUF_LEN*1000/portTICK_PERIOD_MS/freq + 1);
}

///////////////////////////////////////////
// PDMOut  Stop I2S+PDM
void PDMOut_stop()
{
	// wait until output the last data.
  for(int k=0;k<DMA_BUF_SIZE+I2S_FIFO_LEN;k+=2){
	  // dithering
    _WriteOne(0);
    _WriteOne(1);
  }

  // switch to DAC from PDM
	dac_output_voltage(gDacCh, 128);
	dac_output_enable(gDacCh);	//set GPIO connected to analog RTC module.

  // release I2S(PDM)
	i2s_driver_uninstall(i2s_num);

  // keep middle level by DAC
}


///////////////////////////////////////////
//  Write one sample data
// val: a sample data
// return	1:ok 0:Timeout -1:ESP_FAIL
int PDMOut_write(int16_t val)
{
  // add dither
  int16_t dither = esp_random() & 0xff;
  if(val >= -32768+dither) val -= dither;
  return _WriteOne(val);
}

///////////////////////////////////////////
// Clear DMA buffer
void PDMOut_clear()
{
  for(int k=0;k<DMA_BUF_SIZE;k+=2){
      // dithering
      _WriteOne(0);  
      _WriteOne(1);
    }
}

// write witout dither
static int _WriteOne(int16_t val)
{
   // write to I2S DMA buffer
  int16_t sample[2];
  sample[0]=sample[1]=val; // mono -> stereo
  return i2s_push_sample(i2s_num, (const char *)sample, gTicsWait);
}


