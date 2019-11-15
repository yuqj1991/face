#ifndef V4L2CORE_H_INCLUDED
#define V4L2CORE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UVC_LOG 1

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define VIDIOC_REQBUFS_COUNT 2
#define ULOG(pfile,format,args...) \
{\
    if(UVC_LOG)\
    {\
        time_t nowtime = time(NULL);\
        struct tm * p = localtime(&nowtime);\
        char stime[100];\
        strftime(stime, 100, "%Y-%m-%d %H:%M:%S", p);\
        if(pfile)\
        {\
            fprintf(pfile, "%s - " format"\n", stime, ##args);\
            fflush(pfile);\
        }\
    }\
}
#define SAFEFREE(p) \
{\
    if(p)\
    {\
        free(p);\
        p=NULL;\
    }\
}

//sdata

typedef struct DataPktHeader{
    unsigned char   header_magic[8];
    uint32_t        header_len;
    uint32_t        payload_total_len;
    uint32_t        payload_used_len;
    uint32_t        pkt_num;
    uint32_t        pkt_index;
    unsigned char   sub_protocol_type;
    unsigned char   reserve[3];
    uint32_t        checksum;
} DataPktHeader;

#define QUEUE_BUF_SIZE      1024*1000
#define SDATA_PAYLOAD_SIZE  QUEUE_BUF_SIZE-sizeof(DataPktHeader)

typedef struct DataPkt{
    DataPktHeader   header;
    unsigned char   payload[SDATA_PAYLOAD_SIZE];
} DataPkt;

typedef void (*ProcessVBuff)(char* buff,int size);

typedef enum {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
} io_method;

typedef enum {
        FMT_H264,
        FMT_YUV,
        FMT_MJPEG,
} fmt_type;

typedef struct buffer {
        void *                  start;
        size_t                  length;
} buffer;

typedef struct DataBuffer{
    unsigned char*      buf;
    size_t              size;
}DataBuffer;

typedef struct FrameBuffer{
    unsigned char*      buf;
    size_t              size;
    unsigned int        index;
}FrameBuffer;

typedef struct SDataBuffer{
    unsigned char       *buf;
    size_t              size;
    int                 b_CheckSumOK;
}SDataBuffer;

typedef struct v4l2_dev_t{
    //device
    int fd;
    char *deviceName;

    //device mutex
    pthread_mutex_t mutex;

    //UVC
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_format fmtack;
	struct v4l2_streamparm frameint;
	struct v4l2_fmtdesc fmtdesc;
    struct v4l2_frmsizeenum frmsozeenum;

    //video
    unsigned int width;
    unsigned int height;
    unsigned int fps;
    fmt_type     fmtType;

    DataBuffer      img;
    FrameBuffer      frame;
    SDataBuffer      sdata;

    //capture
    io_method    io;
    buffer*      buffers;
    unsigned int n_buffers;

    ProcessVBuff VBuffCallback;
    unsigned int bcapture;

    //result data
    unsigned int breaddata;

    //File
    uint8_t bSaveVideoFile;
    char *videoFileName;
    FILE *videoFile;
    uint8_t bSaveResultFile;
    char *resultFileName;
    FILE *resultFile;

    //heart
    int isHeartEnable;

    int bIOBusy;
#if UVC_LOG
    FILE * logfile;
#endif

} v4l2_dev_t;

unsigned int check_sum(unsigned char *a, int len);

int xioctl(int fd, int request, void* argp);

v4l2_dev_t* v4l2core_dev_open(const char* name);

void v4l2core_dev_clean(v4l2_dev_t *vd);

void v4l2core_dev_init(v4l2_dev_t* vd);

void v4l2core_dev_uninit(v4l2_dev_t* vd);

int v4l2core_capture_start(v4l2_dev_t* vd);

void v4l2core_capture_loop(v4l2_dev_t* vd);

int v4l2core_capture_getframe(v4l2_dev_t* vd);

void v4l2core_capture_stop(v4l2_dev_t* vd);

#ifdef __cplusplus
}
#endif

#endif // V4L2CORE_H_INCLUDED
