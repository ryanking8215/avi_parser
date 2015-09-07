#include "assert.h"
#include <unistd.h>

#include "avi_parser.h"

static void test_media(AVIParser *ap)
{
    AVIMedia am;
    avi_media_init(&am, ap);

    AVIMediaInfo ami;

    int i=0;
    while (1) {
        int ret = avi_media_get_data(&am,&ami,NULL,1000);
        assert(ret==0);
        assert(ami.total_size==62672);
        assert(ami.size = 1000);
        assert(ami.is_video);
        i++;
        if (ami.flag & kAVIMediaFlagLast) {
            assert(i>10);
            printf("%d\n",i);
            break;
        }
    }

    i=0;
    while (1) {
        int ret = avi_media_get_data(&am,&ami,NULL,1000);
        assert(ret==0);
        assert(ami.total_size==256);
        assert(ami.size = 256);
        assert(!ami.is_video);
        i++;
        if (ami.flag & kAVIMediaFlagLast) {
            assert(i==1);
            break;
        }
    }

    avi_media_deinit(&am);

    /*while (1) {*/
        /*int ret = avi_media_get_data(&am,NULL,0);*/
        /*if (ret<0) {*/
            /*break;*/
        /*}*/
        /*sleep(1);*/
    /*}*/
}

int main(void)
{
    const char * file = "test.avi";

    AVIParser ap;
    int ret = avi_parser_parse(&ap, file);
    assert(ret==0);

    printf("stream count:%d\n", ap.main_hdr.streams);
    AVIStreamHeader *sh;
    sh = avi_parser_get_video_header(&ap);
    printf("video length:%d\n", sh->length);

    sh = avi_parser_get_audio_header(&ap);
    printf("audio length:%d\n", sh->length);

    printf("has index:%d\n", avi_parser_has_index_entry(&ap));

    printf("movi entry_offset:0x%x\n", ap.movi_info.entry_offset);
    printf("idxl entry_offset:0x%x\n", ap.idxl_info.entry_offset);

    printf("sec:%d\n",ap.total_sec);

    test_media(&ap);

    return 0;
}
