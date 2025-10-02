#ifndef AVFORMAT_AVC_PRIM_H
#define AVFORMAT_AVC_PRIM_H

#include <stdint.h>
#include "avio.h"

int ff_avc_parse_nal_units(ByteIOContext *s, uint8_t *buf, int size);
int ff_avc_parse_nal_units_buf(uint8_t *buf_in, uint8_t **buf, int *size);
int ff_isom_write_avcc(ByteIOContext *pb, uint8_t *data, int len);
uint8_t *ff_avc_find_startcode(uint8_t *p, uint8_t *end);


#endif


