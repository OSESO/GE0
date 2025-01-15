#ifndef __SOUND_H__
#define __SOUND_H__

#include <stdint.h>

void addTone(uint16_t f, uint16_t t);
void setRtttlAddress(char *adr);
void setRtttlLoop(uint8_t loop);
void setRtttlPlay(uint8_t play);
#endif
