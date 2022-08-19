#ifndef REVERB_H
#define REVERB_H

#include <stdint.h>

uint8_t reverb_init(int M);
void reverb_deinit();

int16_t reverb(int16_t sample, float g_comb0,
               float g_comb1, int16_t m_comb1,
               float g_comb2, int16_t m_comb2,
               float g_comb3, int16_t m_comb3,
               float g_ap0, int16_t m_ap0,
               float g_ap1, int16_t m_ap1,
               float g_ap2, int16_t m_ap2);

#endif /*REVERB_H*/