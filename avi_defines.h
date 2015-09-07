#ifndef _AVI_DEFINES_H
#define _AVI_DEFINES_H

#include <stdint.h>

typedef uint8_t FOURCC[4];

typedef struct AVICommonHeader {
    FOURCC fourcc;
    uint32_t length;
} AVICommonHeader;

typedef struct AVITypeHeader {
    FOURCC fourcc;
    uint32_t length;
    FOURCC type;
} AVITypeHeader;

typedef struct AVIMainHeader {
    uint32_t ms_per_frame;
    uint32_t max_bytes_per_sec;
    uint32_t padding_granularity;
    uint32_t flags;
    uint32_t total_frames;
    uint32_t initial_frames;
    uint32_t streams;
    uint32_t suggested_buffer_size;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[4];
} AVIMainHeader;

typedef struct AVIStreamHeader {
    //FOURCC fourcc; // 必须为'strh'
    //uint32_t length;   // 本数据结构的大小，不包括最初的8个字节（fcc和cb两个域）
    FOURCC type;    // 流的类型：‘auds’（音频流）、‘vids’（视频流）、‘mids’（MIDI流）、‘txts’（文字流）
    FOURCC fcc_handler; // 指定流的处理者，对于音视频来说就是解码器
    uint32_t flags;    // 标记：是否允许这个流输出？调色板是否变化？
    uint16_t priority;  // 流的优先级（当有多个相同类型的流时优先级最高的为默认流）
    uint16_t language;
    uint32_t initial_frames; // 为交互格式指定初始帧数
    uint32_t scale;   // 这个流使用的时间尺度
    uint32_t rate;
    uint32_t start;   // 流的开始时间
    uint32_t length;  // 流的长度（单位与dwScale和dwRate的定义有关）
    uint32_t suggested_buffer_size; // 读取这个流数据建议使用的缓存大小
    uint32_t quality;    // 流数据的质量指标（0 ~ 10,000）
    uint32_t sample_size; // Sample的大小
    struct {
        uint16_t left;
        uint16_t top;
        uint16_t right;
        uint16_t bottom;
    }  rcFrame;  // 指定这个流（视频流或文字流）在视频主窗口中的显示位置
    // 视频主窗口由AVIMAINHEADER结构中的dwWidth和dwHeight决定
} AVIStreamHeader;

typedef struct BitMapInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    int16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BitMapInfoHeader;

typedef struct RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

typedef struct BitMapInfo
{
    BitMapInfoHeader bmi_header;
    RGBQUAD bmi_colors[1]; //颜色表
} BitMapInfo;

typedef struct WaveFormat
{
    uint16_t format_tag;
    uint16_t channels; //声道数
    uint32_t samples_per_sec; //采样率
    uint32_t avg_bytes_per_sec; //WAVE声音中每秒的数据量
    uint16_t block_align; //数据块的对齐标志
    uint16_t size; //此结构的大小
} WaveFormat;

typedef struct
{
    uint32_t ckid; //记录数据块中子块的标记
    uint32_t flags; //表示ckid所指子块的属性
    uint32_t chunk_offset; //子块的相对位置
    uint32_t chunk_length; //子块长度
} AVIIndexEntry;


#endif
