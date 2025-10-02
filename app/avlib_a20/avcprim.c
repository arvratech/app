#include <stdint.h>
#include <assert.h>
#include "intreadwrite.h"
#include "avio.h"
#include "avcprim.h"


static int find_start_code(uint8_t *buf, int buf_size, int *index)
{
	int		i, n;

	i = *index;
	for( ;i+4 < buf_size;i++)
		if(buf[i]==0 && buf[i+1]==0 && (buf[i+2]==1 || buf[i+2]==0 && buf[i+3]==1)) {
		if(buf[i+2]==1) n = 3;
		else	n = 4;
		*index = i + n;
		return n;
	}
    return 0;
}

int ff_isom_write_avcc(ByteIOContext *pb, uint8_t *buf, int len)
{
	uint8_t		*sps, *pps;
	int		n, index, oindex, size, nal_type, sps_size, pps_size;

printf("isom_write: %d\n", len);
	sps_size = pps_size = 0;
	oindex = index = 0;
	n = find_start_code(buf, len, &index);
	while(n >= 3) {
		oindex = index;	
		n = find_start_code(buf, len, &index);
		if(n >= 3) size = index - oindex - n;
		else	  size = len - oindex;
		nal_type = buf[oindex] & 0x1f;
		if(nal_type == 7) {			// SPS
			sps = buf + oindex;
			sps_size = size;
		} else if(nal_type == 8) {	// PPS
			pps = buf + oindex;
			pps_size = size;
		}
	}
	if(sps_size > 0 && pps_size > 0) {
		put_byte(pb, 1);		// version
		put_byte(pb, sps[1]);	// profile
		put_byte(pb, sps[2]);	// profile compat
		put_byte(pb, sps[3]);	// level
		put_byte(pb, 0xff);		// 6 bits reserved (111111) + 2 bits nal size length - 1 (11)
		put_byte(pb, 0xe1);		// 3 bits reserved (111) + 5 bits number of sps (00001)
		put_be16(pb, sps_size);
		put_buffer(pb, sps, sps_size);
		put_byte(pb, 1);		// number of pps
		put_be16(pb, pps_size);
		put_buffer(pb, pps, pps_size);
    }
printf("end...: %d\n", len);
    return 0;
}

