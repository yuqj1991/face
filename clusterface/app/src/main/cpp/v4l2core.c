#include "v4l2core.h"

/**
	Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

	\param fd file descriptor
	\param request request
	\param argp argument
	\returns result from ioctl
*/
int xioctl(int fd, int request, void* argp)
{
	int r;

	do r = ioctl(fd, request, argp);
	while (-1 == r && EINTR == errno);

	return r;
}

static void errno_show(const char* s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
}

static unsigned int get_frame_idx(unsigned char *pic_ptr)
{
    unsigned int frame_idx = 0;

    if (pic_ptr[2] == 0xFF && pic_ptr[3] == 0xE1 && pic_ptr[4] == 0x00 && pic_ptr[5] == 0x12)
        frame_idx = (pic_ptr[10]&0xff)<<24|(pic_ptr[11]&0xff)<<16|(pic_ptr[12]&0xff)<<8|(pic_ptr[13]&0xff)<<0;

    return frame_idx;
}

unsigned int check_sum(unsigned char *a, int len)
{
    unsigned int sum = 0;

    while (len > 0) {
        sum += *a++;
        len --;
    }
    //sum = sum&0xffff;
    return sum;
}

static int mmapInit(v4l2_dev_t *vd)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = VIDIOC_REQBUFS_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(vd->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", vd->deviceName);
			return -1;
		} else {
			errno_show("VIDIOC_REQBUFS");
			return -1;
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", vd->deviceName);
		return -1;
	}

	vd->buffers = calloc(req.count, sizeof(*vd->buffers));

	if (!vd->buffers) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}

	for (vd->n_buffers = 0; vd->n_buffers < req.count; ++vd->n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = vd->n_buffers;

		if (-1 == xioctl(vd->fd, VIDIOC_QUERYBUF, &buf)){
			errno_show("VIDIOC_QUERYBUF");
			return -1;
        }

		vd->buffers[vd->n_buffers].length = buf.length;
		vd->buffers[vd->n_buffers].start = mmap(NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, vd->fd, buf.m.offset);

        printf("V4L2_CORE: mapped buffer[%i] with length %i to pos %p\n",
                        vd->n_buffers,
                        buf.length,
                        vd->buffers[vd->n_buffers].start);

		if (MAP_FAILED == vd->buffers[vd->n_buffers].start){
			errno_show("mmap");
			return -1;
        }
	}
	return 0;
}

v4l2_dev_t* v4l2core_dev_open(const char* deviceName)
{

    v4l2_dev_t* vd = calloc(1,sizeof(v4l2_dev_t));
    assert(vd != NULL);
    memset(vd,0,sizeof(v4l2_dev_t));
    vd->io = IO_METHOD_MMAP;
    vd->fps = 15;
    vd->deviceName = strdup(deviceName);
    vd->width = 0;
    vd->height = 0;
    vd->img.buf=NULL;
    vd->img.size = 0;
    vd->VBuffCallback = NULL;
    vd->bIOBusy = 0;
#if UVC_LOG
    vd->logfile = NULL;
#endif

    struct stat st;
	// stat file
	if (-1 == stat(deviceName, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", vd->deviceName, errno, strerror(errno));
		return NULL;
	}

	// check if its device
	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", vd->deviceName);
		return NULL;
	}

    char cmd[255];
    sprintf(cmd,"su -c 'chmod 666 %s'",deviceName);
    FILE* outstream;
    outstream = popen(cmd,"r");
    if(outstream == NULL)
    {
        fprintf(stderr, "chmod %s error.\n", deviceName);
        return NULL;
    }


    // open device
	vd->fd = open(vd->deviceName, O_RDWR | O_NONBLOCK,0);

	// check if opening was successfull
	if (-1 == vd->fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", vd->deviceName, errno, strerror(errno));
		return NULL;
	}

#if UVC_LOG
    vd->logfile = fopen("uvc_log.txt","at+");
    ULOG(vd->logfile,"*************************LOG*************************");
    ULOG(vd->logfile,"device open,%s.",vd->deviceName);
#endif

	return vd;
}

void v4l2core_dev_clean(v4l2_dev_t *vd)
{
    assert(vd!=NULL);
    if(vd->deviceName)
        free(vd->deviceName);
    vd->deviceName = NULL;
    if(vd->videoFileName)
        free(vd->videoFileName);
    vd->videoFileName = NULL;
    if(vd->resultFileName){
        free(vd->resultFileName);
        }
    vd->resultFileName = NULL;

#if UVC_LOG
    if(vd->logfile)
    {
        ULOG(vd->logfile,"device close.");
        fclose(vd->logfile);
        vd->logfile = NULL;
    }
#endif

    if(vd->fd>0)
    {
        close(vd->fd);
		vd = NULL;
    }
}

void enum_frame_sizes(v4l2_dev_t *vd,uint32_t pixfmt)
{
    puts("\t\tSupport frame size:");
    vd->frmsozeenum.index=0;
    vd->frmsozeenum.pixel_format = pixfmt;
    while(ioctl(vd->fd,VIDIOC_ENUM_FRAMESIZES,&vd->frmsozeenum)!=-1)
    {
        vd->frmsozeenum.index++;
        if (vd->frmsozeenum.type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
            printf("\t\t{ discrete: width = %u, height = %u }\n",
                       vd->frmsozeenum.discrete.width, vd->frmsozeenum.discrete.height);
        }
        else if (vd->frmsozeenum.type == V4L2_FRMSIZE_TYPE_CONTINUOUS || vd->frmsozeenum.type == V4L2_FRMSIZE_TYPE_STEPWISE)
        {
            if(vd->frmsozeenum.type == V4L2_FRMSIZE_TYPE_CONTINUOUS)
                printf("\t\t{ continuous: min { width = %u, height = %u } .. "
                       "max { width = %u, height = %u } }\n",
                       vd->frmsozeenum.stepwise.min_width, vd->frmsozeenum.stepwise.min_height,
                       vd->frmsozeenum.stepwise.max_width, vd->frmsozeenum.stepwise.max_height);
            else
                printf("\t\t{ stepwise: min { width = %u, height = %u } .. "
                       "max { width = %u, height = %u } / "
                       "stepsize { width = %u, height = %u } }\n",
                       vd->frmsozeenum.stepwise.min_width, vd->frmsozeenum.stepwise.min_height,
                       vd->frmsozeenum.stepwise.max_width, vd->frmsozeenum.stepwise.max_height,
                       vd->frmsozeenum.stepwise.step_width, vd->frmsozeenum.stepwise.step_height);
        }
        else
        {
            fprintf(stderr, "V4L2_CORE: fsize.type not supported: %d\n", vd->frmsozeenum.type);
            fprintf(stderr, "    (Discrete: %d   Continuous: %d  Stepwise: %d)\n",
                    V4L2_FRMSIZE_TYPE_DISCRETE,
                    V4L2_FRMSIZE_TYPE_CONTINUOUS,
                    V4L2_FRMSIZE_TYPE_STEPWISE);
        }
    }
}

void v4l2core_dev_init(v4l2_dev_t *vd)
{
    puts("************UVC Device Information************");
    if (-1 == xioctl(vd->fd, VIDIOC_QUERYCAP, &vd->cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",vd->deviceName);
			exit(EXIT_FAILURE);
		} else {
			return;
		}
	}
	else
	{
        printf("driver:\t\t%s\n",vd->cap.driver);
        printf("card:\t\t%s\n",vd->cap.card);
        printf("bus_info:\t%s\n",vd->cap.bus_info);
        printf("version:\t%d\n",vd->cap.version);
        printf("capabilities:\t%x\n",vd->cap.capabilities);

        if ((vd->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
        {
            printf("Device %s: supports capture.\n",vd->deviceName);
        }

        if ((vd->cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
        {
            printf("Device %s: supports streaming.\n",vd->deviceName);
        }

        switch (vd->io) {
		case IO_METHOD_READ:
			if (!(vd->cap.capabilities & V4L2_CAP_READWRITE)) {
				fprintf(stderr, "%s does not support read i/o\n",vd->deviceName);
			}
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
      			if (!(vd->cap.capabilities & V4L2_CAP_STREAMING)) {
				fprintf(stderr, "%s does not support streaming i/o\n",vd->deviceName);
			}
			break;
        }
	}

		/* Select video input, video standard and tune here. */
//	CLEAR(vd->cropcap);

//	vd->cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

//	if (0 == xioctl(vd->fd, VIDIOC_CROPCAP, &vd->cropcap)) {
//		vd->crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//		vd->crop.c = vd->cropcap.defrect; /* reset to default */

//		if (-1 == xioctl(vd->fd, VIDIOC_S_CROP, &vd->crop)) {
//			switch (errno) {
//				case EINVAL:
//					/* Cropping not supported. */
//					break;
//				default:
//					/* Errors ignored. */
//					break;
//			}
//		}
//	} else {
//		/* Errors ignored. */
//	}

    puts("************Support Image Formats Information************");
    //emu all support fmt
    vd->fmtdesc.index=0;
    vd->fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while(ioctl(vd->fd,VIDIOC_ENUM_FMT,&vd->fmtdesc)!=-1)
    {
        printf("\t%d.%s\n",vd->fmtdesc.index+1,vd->fmtdesc.description);
        enum_frame_sizes(vd,vd->fmtdesc.pixelformat);
        vd->fmtdesc.index++;
    }

    puts("************Current Image Formats Information************");
    //set fmt
#if 0
    vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(vd->fmtType == FMT_H264){
        vd->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    }
    else if(vd->fmtType == FMT_YUV){
        vd->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    }
    else if(vd->fmtType == FMT_MJPEG){
        vd->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    }
    vd->fmt.fmt.pix.height = vd->height;
    vd->fmt.fmt.pix.width = vd->width;
    if(vd->fmtType == FMT_YUV)
        vd->fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(xioctl(vd->fd, VIDIOC_S_FMT, &vd->fmt) == -1)
    {
        printf("Unable to set format\n");
        return;
    }
#endif
#if 1//wangkai ceshi
    vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MPEG;
    vd->fmt.fmt.pix.height = 720;
    vd->fmt.fmt.pix.width = 1280;
    vd->fmt.fmt.pix.field = V4L2_FIELD_ANY;
    if(xioctl(vd->fd, VIDIOC_S_FMT, &vd->fmt) == -1)
    {
        printf("Unable to set format\n");
    }
    printf("Set format success.\n");
#endif
    //get fmt
    vd->fmtack.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(xioctl(vd->fd, VIDIOC_G_FMT, &vd->fmtack) == -1)
    {
        errno_show("VIDIOC_G_FMT");
        printf("Unable to get format\n");
        return;
    }
    else
    {
         printf("fmt.type:\t\t%d\n",vd->fmtack.type);
         printf("pix.pixelformat:\t%c%c%c%c\n",vd->fmtack.fmt.pix.pixelformat & 0xFF, (vd->fmtack.fmt.pix.pixelformat >> 8) & 0xFF,(vd->fmtack.fmt.pix.pixelformat >> 16) & 0xFF, (vd->fmtack.fmt.pix.pixelformat >> 24) & 0xFF);
         printf("pix.width:\t\t%d\n",vd->fmtack.fmt.pix.width);
         printf("pix.height:\t\t%d\n",vd->fmtack.fmt.pix.height);
         printf("pix.field:\t\t%d\n",vd->fmtack.fmt.pix.field);

         vd->width = vd->fmtack.fmt.pix.width;
         vd->height = vd->fmtack.fmt.pix.height;

         if(vd->fmtack.fmt.pix.pixelformat == V4L2_PIX_FMT_H264)
         {
             vd->fmtType = FMT_H264;
         }
         else if(vd->fmtack.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG)
         {
             vd->fmtType = FMT_MJPEG;
         }
         else if(vd->fmtack.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420)
         {
             vd->fmtType = FMT_YUV;
         }
    }
    //set fps
#if 0
    if (vd->fps != -1)
    {
        CLEAR(vd->frameint);
        /* Attempt to set the frame interval. */
        vd->frameint.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vd->frameint.parm.capture.timeperframe.numerator = 1;
        vd->frameint.parm.capture.timeperframe.denominator = vd->fps;
        if (-1 == xioctl(vd->fd, VIDIOC_S_PARM, &vd->frameint))
          fprintf(stderr,"Unable to set frame interval.\n");
    }
#endif
    //get fps
    CLEAR(vd->frameint);
    vd->frameint.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(vd->fd, VIDIOC_G_PARM, &vd->frameint)){
        fprintf(stderr,"Unable to set frame interval.\n");
    }
    else
    {
        printf("capture fps:\t\t%d/%d\n",vd->frameint.parm.capture.timeperframe.numerator,
               vd->frameint.parm.capture.timeperframe.denominator);
        vd->fps = vd->frameint.parm.capture.timeperframe.denominator/vd->frameint.parm.capture.timeperframe.numerator;
    }

    switch (vd->io) {
		case IO_METHOD_READ:
			//readInit(fmt.fmt.pix.sizeimage);
			break;
		case IO_METHOD_MMAP:
			mmapInit(vd);
			break;
		case IO_METHOD_USERPTR:
			//userptrInit(fmt.fmt.pix.sizeimage);
			break;
	}

	//init device mutex
	pthread_mutex_init(&vd->mutex,NULL);
}

void v4l2core_dev_uninit(v4l2_dev_t* vd)
{
    unsigned int i;
	switch (vd->io) {
		case IO_METHOD_READ:
			free(vd->buffers[0].start);
			break;
		case IO_METHOD_MMAP:
			for (i = 0; i < vd->n_buffers; ++i)
				if (-1 == munmap(vd->buffers[i].start, vd->buffers[i].length))
					errno_show("munmap");
			break;
		case IO_METHOD_USERPTR:
			for (i = 0; i < vd->n_buffers; ++i)
				free(vd->buffers[i].start);
			break;
	}
	free(vd->buffers);
	pthread_mutex_destroy(&vd->mutex);
}

int v4l2core_capture_start(v4l2_dev_t* vd)
{
    if(vd->bSaveVideoFile){
        char* savefname="";
        if(vd->fmtType == FMT_H264)
            savefname = "out.264";
        else if(vd->fmtType == FMT_YUV)
            savefname = "out.yuv";
        else if(vd->fmtType == FMT_MJPEG)
        {
            savefname = "out.avi";
        }
        vd->videoFile = fopen( savefname, "wb" );
        if (!vd->videoFile) {
            vd->videoFile = NULL;
            puts("Can't open save file.");
            return -1;
        }
    }
    printf("v4l2core_capture_start:\tn_buffers:%d\n",vd->n_buffers);
	unsigned int i;
	enum v4l2_buf_type type;

	switch (vd->io) {
		case IO_METHOD_READ:
			/* Nothing to do. */
			break;
		case IO_METHOD_MMAP:
			for (i = 0; i < vd->n_buffers; ++i) {
                printf("v4l2core_capture_start:\tn_buffers:%d\n",i);
				struct v4l2_buffer buf;

				CLEAR(buf);

				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;

				if (-1 == xioctl(vd->fd, VIDIOC_QBUF, &buf)){
                    errno_show("VIDIOC_QBUF");
					return -1;
                }
            }

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == xioctl(vd->fd, VIDIOC_STREAMON, &type)){
                errno_show("VIDIOC_STREAMON");
				return -1;
            }

			break;
		case IO_METHOD_USERPTR:
			for (i = 0; i < vd->n_buffers; ++i) {
				struct v4l2_buffer buf;

			CLEAR (buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long) vd->buffers[i].start;
			buf.length = vd->buffers[i].length;

			if (-1 == xioctl(vd->fd, VIDIOC_QBUF, &buf))
				return -1;
			}

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == ioctl(vd->fd, VIDIOC_STREAMON, &type))
				return -1;
			break;
	}
	return 0;
}

//static int insave = 10;
/**
	process orignal raw data
*/
DataPktHeader dataPktHeader;
uint8_t        *pHeader = (uint8_t*)&dataPktHeader;

static int dataProcess(v4l2_dev_t* vd,const void* p,int len ,struct timeval timestamp)
{
    if(vd->bSaveVideoFile&&vd->videoFile)
    {
        unsigned char* src = (unsigned char*)p;
        fwrite(src,len,1,vd->videoFile);
    }
    unsigned char* pdata = (unsigned char*)p;
    if(pdata[0]==0xFF&&pdata[1]==0xD8&&pdata[2]==0xFF&&pdata[3]==0xE1)//mjpeg data
    {
        unsigned int findex = get_frame_idx(pdata);
        //printf("video recive,\tindex:%d siez:%d\n",findex,len);
        return 1;
    }
    else {
        memcpy(pHeader,p,sizeof(DataPktHeader));
        if(dataPktHeader.header_magic[0]==0xFF&&dataPktHeader.header_magic[1]==0x55
                &&dataPktHeader.header_magic[2]==0xFF
                &&dataPktHeader.header_magic[3]==0xAA)
        {
            printf(" data recive:\t%d\n",len);
            return 2;
        }
        else
        {
            printf("other recive:\t%d\n",len);
            return 0;
        }

    }
}

/**
	read single frame
*/
static int frameRead(v4l2_dev_t* vd)
{
	struct v4l2_buffer buf;
	unsigned int i;

    int retValue = 1;

	switch (vd->io) {
		case IO_METHOD_READ:
			if (-1 == read(vd->fd, vd->buffers[0].start, vd->buffers[0].length)) {
				switch (errno) {
					case EAGAIN:
						return 0;

					case EIO:
						// Could ignore EIO, see spec.
						// fall through

					default:
						return -1;
				}
			}

			/*struct timespec ts;
			struct timeval timestamp;
			clock_gettime(CLOCK_MONOTONIC,&ts);
			timestamp.tv_sec = ts.tv_sec;
			timestamp.tv_usec = ts.tv_nsec/1000;*/

			//imageProcess(vd->buffers[0].start,timestamp);
            retValue = dataProcess(vd,vd->buffers[buf.index].start,buf.bytesused,buf.timestamp);
			break;
		case IO_METHOD_MMAP:
			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
            //puts("VIDIOC_DQBUF");
			if (-1 == xioctl(vd->fd, VIDIOC_DQBUF, &buf)) {
				switch (errno) {
					case EAGAIN:
						return 0;

					case EIO:
						// Could ignore EIO, see spec
						// fall through

					default:
                        errno_show("VIDIOC_DQBUF");
						return -1;
				}
			}

			assert(buf.index < vd->n_buffers);
            retValue = dataProcess(vd,vd->buffers[buf.index].start,buf.bytesused,buf.timestamp);

            if(retValue==1){
                if(vd->VBuffCallback){
                    vd->VBuffCallback((char*)vd->buffers[buf.index].start,buf.bytesused);
                }
                vd->frame.buf = (unsigned char*)vd->buffers[buf.index].start;
                vd->frame.size = buf.bytesused;
                vd->frame.index = get_frame_idx((unsigned char*)vd->buffers[buf.index].start);
            }
            else if(retValue==2)
            {
                //memcpy(vd->sdata.buf,(unsigned char*)vd->buffers[buf.index].start,buf.bytesused);
                vd->sdata.buf = (unsigned char*)vd->buffers[buf.index].start;
                vd->sdata.size = buf.bytesused;
                vd->sdata.size = buf.bytesused;
            }

			if (-1 == xioctl(vd->fd, VIDIOC_QBUF, &buf))
                errno_show("VIDIOC_QBUF");

			break;
        case IO_METHOD_USERPTR:
				CLEAR (buf);

				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_USERPTR;

				if (-1 == ioctl(vd->fd, VIDIOC_DQBUF, &buf)) {
					switch (errno) {
						case EAGAIN:
							return 0;

						case EIO:
							// Could ignore EIO, see spec.
							// fall through

						default:
							return -1;
					}
				}

				for (i = 0; i < vd->n_buffers; ++i)
					if (buf.m.userptr == (unsigned long)vd->buffers[i].start && buf.length == vd->buffers[i].length)
						break;

				assert (i < vd->n_buffers);

				//imageProcess((void *)buf.m.userptr,buf.timestamp);
                retValue = dataProcess(vd,vd->buffers[buf.index].start,buf.bytesused,buf.timestamp);

				if (-1 == ioctl(vd->fd, VIDIOC_QBUF, &buf))
					return -1;
				break;
	}
    return retValue;
}

void v4l2core_capture_loop(v4l2_dev_t* vd)
{
    fd_set fds;
    struct timeval tv;
    int r;

    while (vd->bcapture) {

//        if(vd->bIOBusy)
//        {
//            usleep(1000*500);
//            continue;
//        }

        FD_ZERO(&fds);
        FD_SET(vd->fd, &fds);

        /* Timeout. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        r = select(vd->fd + 1, &fds, NULL, NULL, &tv);
        if(r<0)
        {
            fprintf(stderr, "Could not grab image (select error): %s\n", strerror(errno));
        }
        else if(r==0)
        {
            fprintf(stderr, "Could not grab image (select timeout): %s\n", strerror(errno));
        }
        else if(r>0)
        {
            if(vd->bIOBusy)
                vd->bIOBusy = 0;
            frameRead(vd);
        }
	}
}

int v4l2core_capture_getframe(v4l2_dev_t* vd)
{

    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(vd->fd, &fds);

    /* Timeout. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select(vd->fd + 1, &fds, NULL, NULL, &tv);
    if(r<0)
    {
        fprintf(stderr, "Could not grab image (select error): %s\n", strerror(errno));
        return -1;
    }
    else if(r==0)
    {
        fprintf(stderr, "Could not grab image (select timeout): %s\n", strerror(errno));
        return -1;
    }

    return frameRead(vd);
}


void v4l2core_capture_stop(v4l2_dev_t* vd)
{
    enum v4l2_buf_type type;

	switch (vd->io) {
		case IO_METHOD_READ:
			/* Nothing to do. */
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == ioctl(vd->fd, VIDIOC_STREAMOFF, &type))
			puts("VIDIOC_STREAMOFF error.");
			break;
	}

    if (vd->videoFile) {
        fclose(vd->videoFile);
        vd->videoFile = NULL;
	}
}
