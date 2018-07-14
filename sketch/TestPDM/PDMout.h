#if !defined(_PDMOUT_H_)
#define _PDMOUT_H_
#ifdef __cplusplus
extern "C"{
#endif

// gpio: output pin. only  25 or 26 
void PDMOut_create(int gpio);
void PDMOut_release();

// frequency: sampling rate (ex: 8000, 44100…) [Hz]
void PDMOut_start(int frequency);
void PDMOut_stop();

int PDMOut_write(int16_t val);
void PDMOut_clear();

#ifdef __cplusplus
}
#endif
#endif // !defined(_PDMOUT_H_)
