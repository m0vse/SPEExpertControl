#ifndef EXPERTPACKETS_H
#define EXPERTPACKETS_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DATA 30

struct __attribute__((packed)) Expert_Packet {
  uint8_t status_code;
  uint8_t flags;
  uint8_t display_ctx;
  uint8_t setup[11];
  uint8_t band_input;
  uint8_t sub_band;
  uint16_t freq;
  uint8_t antenna_cat;
  uint16_t swr_gain;
  uint8_t temp;
  uint16_t power;
  uint16_t rev_power;
  uint16_t voltage;
  uint16_t current;      
};


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif