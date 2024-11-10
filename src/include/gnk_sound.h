#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

typedef struct{
    uint32_t stereo;
    uint32_t sample_rate;
    uint32_t track_size; //cant tell if used at all
    uint32_t padded_size;
    char name[0x40];
} music_track;

typedef struct{
    uint32_t header;
    uint32_t version;
    uint32_t interleave;
    uint32_t size;
    uint32_t sample_rate;
    char padding[0x0a];
    uint8_t channels;
    char padding2[0x11];
} VAGp;

typedef struct{
    //sshd
    uint32_t sshd_header;
    uint32_t sshd_header_size;
    uint32_t data_type;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t interleave;
    uint64_t ff_footer;

    //ssbd
    uint32_t ssbd_header;
    uint32_t adpcm_size;
} ADS;

typedef struct{
    char SCEIVers_header[0x8];
    uint32_t SCEIVers_size;
    uint32_t SCEIVers_version;
} SCEIVers;

typedef struct{
    char SCEIHead_header[0x8];
    uint32_t SCEIHead_size;
    uint32_t SCEI_size;
    uint32_t ADPCM_size;
    uint32_t SCEIProg_offset;
    uint32_t SCEISet_offset;
    uint32_t SCEISmpl_offset;
    uint32_t SCEIVagi_offset;
} SCEIHead;

typedef struct{
    uint32_t start_offset;
    uint16_t sample_rate;
    uint8_t looping;
    uint8_t ff;
} SCEIVagiData;

typedef struct{
    char SCEIVagi_header[0x8];
    uint32_t SCEIVagi_size;
    uint32_t data_count;
} SCEIVagi;

static const unsigned char SShd_HEADER[] = { //used in ADS
    0x53, 0x53, 0x68, 0x64
};
static const unsigned char SSbd_HEADER[] = { //used in ADS
    0x53, 0x53, 0x62, 0x64
};
static const unsigned char VAGp_HEADER[] = { //VAGp mono header
    0x56, 0x41, 0x47, 0x70, 0x00, 0x00, 0x00, 0x20
};
static const unsigned char VAGi_HEADER[] = { //VAGp mono header
    0x56, 0x41, 0x47, 0x69, 0x00, 0x00, 0x00, 0x20
};
static const unsigned char BD[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char HD[8] = {
    0x49, 0x45, 0x43, 0x53, 0x73, 0x72, 0x65, 0x56
};

uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

void write_ads_header(FILE *out, uint32_t sample_rate, uint32_t size, uint32_t interleave, uint8_t channels)
{
    ADS ads;
    ads.sshd_header = *(uint32_t *)SShd_HEADER;
    ads.sshd_header_size = 0x18;
    ads.data_type = 0x10;
    ads.sample_rate = sample_rate;
    ads.channels = channels;
    ads.interleave = interleave;
    ads.ff_footer = 0xffffffffffffffff;
    ads.ssbd_header = *(uint32_t *)SSbd_HEADER;
    ads.adpcm_size = size;
    fwrite(&ads, sizeof(ADS), 1, out);
}

void write_vagp_header(FILE *out, uint32_t sample_rate, uint32_t size)
{
    char *padding;
    padding = calloc(0x40, 1);
    fwrite(VAGp_HEADER, 1, sizeof(VAGp_HEADER), out);
    fwrite(padding, 1, 4, out);
    size = swap_uint32(size);
    sample_rate = swap_uint32(sample_rate);
    fwrite(&size, 4, 1, out);
    fwrite(&sample_rate, 4, 1, out);
    fwrite(padding, 1, 0x0a, out);
    fwrite(padding, 1, 1, out);
    fwrite(padding, 1, 0x11, out);
    free(padding);
}

