#include <stdio.h>
#include <string.h>

#include "avi_parser.h"

int avi_parser_parse(AVIParser *p, const char *filename)
{
    if (p==NULL || filename==NULL) {
        return -1;
    }
    memset(p,0,sizeof(*p));
    p->video.stream_id = -1;
    p->audio.stream_id = -1;

    strcpy(p->filename, filename);

    FILE *fp = fopen(filename,"rb");
    if (fp==NULL) {
        return -1;
    }

    int ret = 0;
    AVITypeHeader riff;
    if (fread(&riff,sizeof(riff),1,fp) != 1) {
        goto fail;
    }
    if (memcmp(riff.fourcc,"RIFF",4)!=0) {
        goto fail;
    }
    memcpy(p->file_type,riff.type,4);

    AVITypeHeader hdrl;
    if (fread(&hdrl,sizeof(hdrl),1,fp) != 1) {
        goto fail;
    }
    if (memcmp(hdrl.fourcc,"LIST",4)!=0 || memcmp(hdrl.type,"hdrl",4)!=0) {
        goto fail;
    }

#if 1
    // avih
    AVICommonHeader avih;
    if (fread(&avih, sizeof(avih), 1, fp)!=1) {
        goto fail;
    }
    if (memcmp(avih.fourcc,"avih",4)!=0) {
        goto fail;
    }
    AVIMainHeader main_hdr;
    if (avih.length != sizeof(main_hdr)) {
        goto fail;
    }
    if (fread(&main_hdr,sizeof(main_hdr),1,fp)!=1) {
        goto fail;
    }
    p->main_hdr = main_hdr;
#endif

    // stream list
    int i;
    int is_video;
    int sl_pos;
    for (i=0; i<main_hdr.streams; i++) {
        is_video = 0;

        sl_pos = ftell(fp);

        AVITypeHeader strl;
        if (fread(&strl,sizeof(strl),1,fp)!=1) {
            goto fail;
        }
        if (memcmp(strl.fourcc,"LIST",4)!=0 || memcmp(strl.type,"strl",4)!=0) {
            goto fail;
        }

        AVICommonHeader strh;
        if (fread(&strh,sizeof(strh),1,fp)!=1) {
            goto fail;
        }
        if (memcmp(strh.fourcc,"strh",4)!=0) {
            goto fail;
        }
        AVIStreamHeader stream_hdr;
        if (fread(&stream_hdr, sizeof(stream_hdr),1,fp)!=1) {
            goto fail;
        }
        if (memcmp(stream_hdr.type,"vids",4)==0) {
            p->video.stream_id = i;
            p->stream_hdrs[i] = stream_hdr;
            is_video = 1;
        }
        else if (memcmp(stream_hdr.type,"auds",4)==0) {
            p->audio.stream_id = i;
            p->stream_hdrs[i] = stream_hdr;
        }
        else {
            goto fail;
        }

        AVICommonHeader strf;
        if (fread(&strf,sizeof(strf),1,fp)!=1) {
            goto fail;
        }
        if (is_video) {
            BitMapInfoHeader bmih;
            if (fread(&bmih,sizeof(bmih),1,fp)!=1) {
                goto fail;
            }
            p->video.bitmap_info = bmih;
        }
        else {
            WaveFormat wf;
            if (fread(&wf,sizeof(wf),1,fp)!=1) {
                goto fail;
            }
            p->audio.wave_format = wf;
        }

        // 首先跳到strl的起始处，然后再跳到strl结束的地方
        fseek(fp, sl_pos, SEEK_SET);
        fseek(fp, 8+strl.length, SEEK_CUR);
    }

    // movi
    AVITypeHeader movi;
    p->movi_info.offset = ftell(fp);
    if (fread(&movi,sizeof(movi),1,fp)!=1) {
        goto fail;
    }
    p->movi_info.entry_offset = ftell(fp);
    p->movi_info.size = movi.length - 4;

    // idxl
    if (fseek(fp, p->movi_info.offset+8+movi.length, SEEK_SET)==0) {
        AVITypeHeader idxl;
        p->idxl_info.offset = ftell(fp);
        if (fread(&idxl,sizeof(idxl),1,fp)!=1) {
            goto fail;
        }
        p->idxl_info.entry_offset = ftell(fp);
        p->idxl_info.size = idxl.length - 4;
    }

    p->total_sec = avi_video_calc_seconds(avi_parser_get_video_header(p));
    goto end;

fail:
    ret=-1;
end:
    fclose(fp);
    return ret;
}

AVIStreamHeader * avi_parser_get_video_header(AVIParser *p)
{
    if (p==NULL) return NULL;
    if (p->video.stream_id <0) {
        return NULL;
    }
    return &p->stream_hdrs[p->video.stream_id];
}

BitMapInfoHeader * avi_parser_get_video_format(AVIParser *p)
{
    if (p==NULL) return NULL;
    if (p->video.stream_id<0) {
        return NULL;
    }
    return &p->video.bitmap_info;
}

AVIStreamHeader * avi_parser_get_audio_header(AVIParser *p)
{
    if (p==NULL) return NULL;
    if (p->audio.stream_id <0) {
        return NULL;
    }
    return &p->stream_hdrs[p->audio.stream_id];
}

WaveFormat * avi_parser_get_audio_format(AVIParser *p)
{
    if (p==NULL) return NULL;
    if (p->audio.stream_id <0) {
        return NULL;
    }
    return &p->audio.wave_format;
}

int avi_parser_has_index_entry(AVIParser *p)
{
    return p->idxl_info.size>0?1:0;
}

int avi_video_calc_seconds(AVIStreamHeader *vh)
{
    if (vh==NULL) return 0;
    if (vh->scale<=0) {
        vh->scale = 1;
    }
    int rate = vh->rate/vh->scale;
    return rate<=0?0:vh->length/rate;
}

int avi_media_init(AVIMedia *media, AVIParser *parser)
{
    if (media==NULL || parser==NULL) {
        return -1;
    }
    memset(media,0,sizeof(*media));
    media->parser = parser;
    media->data_fp = fopen(parser->filename, "rb");
    if (media->data_fp) {
        fseek(media->data_fp, parser->movi_info.entry_offset, SEEK_SET);
    }
    if (avi_parser_has_index_entry(parser)) {
        media->idx_fp = fopen(parser->filename, "rb");
        if (media->idx_fp) {
            fseek(media->idx_fp, parser->idxl_info.entry_offset, SEEK_SET);
        }
    }
    return 0;
}

void avi_media_deinit(AVIMedia *media)
{
    if (media) {
        if (media->data_fp) {
            fclose(media->data_fp);
        }
        if (media->idx_fp) {
            fclose(media->idx_fp);
        }
    }
}

int avi_media_get_data(AVIMedia *media, AVIMediaInfo *info, char *buf, int buf_len)
{
    if (media == NULL || buf_len<=0 || info==NULL) {
        return -1;
    }
    FILE *fp = media->data_fp;
    int ret;
    memset(info,0,sizeof(*info));

    if (media->cur_left_size==0) {
        ret = fread(&media->cur_movi_hdr, sizeof(AVICommonHeader),1, fp);
        if (ret!=1) {
            return -2;
        }
        /*media->cur_total_size = media->cur_movi_hdr.length;*/
        printf("%c%c%c%c length:%d\n", media->cur_movi_hdr.fourcc[0], media->cur_movi_hdr.fourcc[1], media->cur_movi_hdr.fourcc[2], media->cur_movi_hdr.fourcc[3], media->cur_movi_hdr.length);

        media->cur_total_size = media->cur_movi_hdr.length;
        media->cur_left_size = media->cur_total_size;
        if ((media->cur_movi_hdr.fourcc[1]-'0')==media->parser->video.stream_id) {
            media->is_video = 1;
        }
        else {
            media->is_video = 0;
        }

        info->flag |= kAVIMediaFlagFirst;
    }

    info->movi_hdr = media->cur_movi_hdr;
    info->total_size = media->cur_total_size;
    info->is_video = media->is_video;

    if (buf_len > media->cur_left_size) {
        buf_len = media->cur_left_size;
    }
    if (buf!=NULL) {
        if (fread(buf,buf_len,1,fp)==1) {
            ret=0;
        }
        else {
            ret=-1;
        }
    }
    else { // consume
        ret = fseek(fp,buf_len,SEEK_CUR);
    }

    if (ret==0) {
        info->size = buf_len;
        media->cur_left_size-=buf_len;
    }
    if (media->cur_left_size==0) {
        info->flag |= kAVIMediaFlagLast;
    }

    return 0;
}
