#ifndef _AVI_PARSER_H
#define _AVI_PARSER_H

#include <stdio.h>

#include "avi_defines.h"

#define MAX_STREAM_NUM 2

typedef struct ChunkInfo {
    uint32_t offset;
    uint32_t entry_offset;
    uint32_t size;
} ChunkInfo;

typedef struct StreamInfo {
    int stream_id;
    union {
        BitMapInfoHeader bitmap_info;
        WaveFormat  wave_format;
    };
} StreamInfo;

typedef struct {
    char filename[64];
    char file_type[5];
    uint32_t total_sec;
    AVIMainHeader main_hdr;
    AVIStreamHeader stream_hdrs[MAX_STREAM_NUM];
    StreamInfo video;
    StreamInfo audio;
    ChunkInfo movi_info;
    ChunkInfo idxl_info;
} AVIParser;

int avi_parser_parse(AVIParser *p, const char *filename);

AVIStreamHeader * avi_parser_get_video_header(AVIParser *p);
BitMapInfoHeader * avi_parser_get_video_format(AVIParser *p);

AVIStreamHeader * avi_parser_get_audio_header(AVIParser *p);
WaveFormat * avi_parser_get_audio_format(AVIParser *p);

int avi_parser_has_index_entry(AVIParser *p);

int avi_video_calc_seconds(AVIStreamHeader *vh);

typedef struct {
    AVIParser *parser;
    FILE *data_fp;
    FILE *idx_fp;

    AVICommonHeader cur_movi_hdr;
    uint32_t cur_total_size;
    uint32_t cur_left_size;
    int is_video;
} AVIMedia;

enum AVIMediaFlag {
    kAVIMediaFlagFirst = 1<<0,
    kAVIMediaFlagLast = 1<<1,
};

typedef struct {
    AVICommonHeader movi_hdr;
    uint32_t total_size;
    int is_video;
    int flag;
    uint32_t size;
} AVIMediaInfo;

int avi_media_init(AVIMedia *media, AVIParser *parser);
void avi_media_deinit(AVIMedia *media);
int avi_media_get_data(AVIMedia *media, AVIMediaInfo *info, char *buf, int buf_len);
int avi_media_locate(AVIMedia *media, uint32_t sec);

#endif
