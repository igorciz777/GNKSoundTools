#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

typedef struct{
    uint32_t type; //?
    uint32_t sample_rate;
    uint32_t track_size;
    uint32_t padded_size;
    char name[0x40];
} music_track;

uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

static const unsigned char VAG_HEADER[] = { //VAGi header for 2channel interleaved stereo
    0x56, 0x41, 0x47, 0x69, 0x00, 0x00, 0x00, 0x20
};
static const unsigned char BD[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};