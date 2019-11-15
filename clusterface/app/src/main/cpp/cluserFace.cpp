#include <jni.h>
#include <string>
#include <android/log.h>
#include </home/deepano/workspace/clusterFace/app/libs/libFVFace/include/face_lib.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "v4l2core.h"
//#include "mjpeg_decoder.h"
#include "mjpg_decode2.h"
#include "/home/deepano/workspace/clusterFace/app/libs/libFVFace/include/face_lib.h"
#include <android/bitmap.h>

#define TAG "native-jni"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

v4l2_dev_t* vd = NULL;
pthread_t tid_capture;

jmethodID dataCallback;
static jobject* gCallObj = NULL;
JavaVM* gJVM = NULL;
JNIEnv *gEnv = NULL;
jint gVersion = 0;

bool GetJniEnv(JavaVM *vm, JNIEnv **env) {
    bool did_attach_thread = false;
    *env = NULL;
    // Check if the current thread is attached to the VM
    jint get_env_result = vm->GetEnv((void**)env, JNI_VERSION_1_6);
    if (get_env_result == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(env, NULL) == JNI_OK) {
            did_attach_thread = true;
        } else {
            // Failed to attach thread. Throw an exception if you want to.
        }
    } else if (get_env_result == JNI_EVERSION) {
        // Unsupported JNI version. Throw an exception if you want to.
    }
    return did_attach_thread;
}

void invokeDataCallback(uint8_t* buf, int size, int frameNum , int dataType)
{
    if(!gJVM)
        return;
    JNIEnv *env;
    bool bret = GetJniEnv(gJVM,&env);
    //int ret = gJVM->GetEnv((void**)&env,gVersion/*JNI_VERSION_1_6*/);
    jbyte* buff = (jbyte*)buf;
    jbyteArray jarray = env->NewByteArray(size);
    jbyte buff2[size];
    memcpy(buff2,buf,size);
    env->SetByteArrayRegion(jarray, 0, size, buff);
    //jclass callBackCls = env->FindClass("MyPackage/MyClassCallBacks");
//    jobject obj;
//    env->GetObjectClass(obj);
    env->CallVoidMethod(*gCallObj, dataCallback, jarray, size, frameNum,dataType);
    //env->DeleteLocalRef(jarray);
    //env->ReleaseByteArrayElements(jarray,buff2,0);
}


void* thread_task_capture(void*)
{
    if(vd->img.buf)
    {
        free(vd->img.buf);
        vd->img.buf = NULL;
    }
    if(vd->width<=0||vd->height<=0)
    {
        puts("UVCCaptureThread:no width or width.");
        goto STARTERR;
    }
    vd->img.size = vd->width*vd->height*3;
    vd->img.buf = (unsigned char*)malloc(vd->img.size);
    if(!vd->img.buf)
    {
        puts("UVCCaptureThread:img buffer malloc error.");
        goto STARTERR;
    }

    int ret;
    ret = v4l2core_capture_start(vd);
    if(ret<0)
    {
        goto STARTERR;
    }

    while(vd->bcapture)
    {
        ret = v4l2core_capture_getframe(vd);
        if(ret<=0)
            continue;

        if(ret==2) {
            //TODO:add custom data
            continue;
        }
        LOGD("get frame in jni");
        //invokeDataCallback((uint8_t *)vd->frame.buf,vd->frame.size,vd->frame.index,0);
//        uint8_t * fbuf = (uint8_t*)malloc(vd->frame.size);
//        if(fbuf)
//        {
//            memcpy(fbuf,vd->frame.buf,vd->frame.size);
//            invokeDataCallback((uint8_t *)fbuf,vd->frame.size,vd->frame.index,0);
//        }
    }

    v4l2core_capture_stop(vd);

STARTERR:
    SAFEFREE(vd->img.buf);
    pthread_exit(NULL);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clusterface_MainActivity_init(
        JNIEnv *env,
        jobject obj) {
    env->GetJavaVM(&gJVM);
    gVersion = env->GetVersion();
    jclass clazz = env->GetObjectClass(obj);
    dataCallback = env->GetMethodID(clazz, "onDataRecv", "([BIII)V");
    gCallObj = &obj;
    return;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_clusterface_MainActivity_openUVCDev(
        JNIEnv *env,
        jobject /* this */,
        jstring dname) {

    const char *c_dname = env->GetStringUTFChars(dname, JNI_FALSE);
    //open device
    vd = v4l2core_dev_open(c_dname);
    env->ReleaseStringUTFChars(dname, c_dname);
    if(vd == NULL)
    {
        puts("Can't open video device.");
        return errno;
    }
    v4l2core_dev_init(vd);
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clusterface_MainActivity_closeUVCDev(
        JNIEnv *env,
        jobject /* this */) {

    if(vd==NULL)
        return;
    vd->bcapture = 0;
    v4l2core_dev_uninit(vd);
    v4l2core_dev_clean(vd);
    vd = NULL;
    return;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_deepano_facereco_common_ARNativeHelper_getCamWidth(
        JNIEnv *env,
        jobject /* this */) {

    if(!vd)
    {
        return -1;
    }
    return vd->width;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_clusterface_MainActivity_getCamHigh(
        JNIEnv *env,
        jobject /* this */) {

    if(!vd)
    {
        return -1;
    }
    return vd->height;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_clusterface_MainActivity_startCapture(
        JNIEnv *env,
        jobject obj/* this */) {

    int ret =0;
    vd->bcapture = 1;
//    gEnv = env;
//    ret = pthread_create(&tid_capture,NULL,thread_task_capture,NULL);
//    if(ret)
//    {
//        puts("create capture thread error!");
//        return -1;
//    }

    if(vd->width<=0||vd->height<=0)
    {
        puts("UVCCaptureThread:no width or width.");
        return -1;
    }
//    vd->img.size = vd->width*vd->height*3;
//    vd->img.buf = (unsigned char*)malloc(vd->img.size);
//    if(!vd->img.buf)
//    {
//        puts("UVCCaptureThread:image buffer malloc error.");
//        return -1;
//    }
//    ret = MJPGDecoder_init();
//    if(ret<0)
//    {
//        return -1;
//    }

    //int ret;
    ret = v4l2core_capture_start(vd);
    if(ret<0)
    {
        return ret;
    }
    return 0;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_clusterface_MainActivity_getFrame(
        JNIEnv *env,
        jobject obj/* this */, jobject retObj) {
    if (!vd || vd->bcapture == 0)
        return NULL;
    int ret = -1;
    ret = v4l2core_capture_getframe(vd);
    char error[255];
    if (ret == 1) {
        jbyteArray jarray = env->NewByteArray(vd->frame.size);
        env->SetByteArrayRegion(jarray, 0, vd->frame.size, (jbyte *) vd->frame.buf);
        jclass clazz = env->GetObjectClass(retObj);
        jfieldID param_size = env->GetFieldID(clazz, "size", "I");
        jfieldID param_index = env->GetFieldID(clazz, "frame_index", "I");
        jfieldID param_type = env->GetFieldID(clazz, "data_type", "I");
        env->SetIntField(retObj, param_size, vd->frame.size);
        env->SetIntField(retObj, param_index, vd->frame.index);
        env->SetIntField(retObj, param_type, 1);
        env->DeleteLocalRef(clazz);
        //env->DeleteLocalRef(jarray);
        return jarray;
    } else if (ret == 2) {
        jbyteArray jarray = env->NewByteArray(vd->sdata.size);
        env->SetByteArrayRegion(jarray, 0, vd->sdata.size, (jbyte *) vd->sdata.buf);
        jclass clazz = env->GetObjectClass(retObj);
        jfieldID param_size = env->GetFieldID(clazz, "size", "I");
        jfieldID param_index = env->GetFieldID(clazz, "frame_index", "I");
        jfieldID param_type = env->GetFieldID(clazz, "data_type", "I");
        env->SetIntField(retObj, param_size, vd->sdata.size);
        env->SetIntField(retObj, param_index, 0);
        env->SetIntField(retObj, param_type, 2);
        env->DeleteLocalRef(clazz);
        //env->DeleteLocalRef(jarray);
        return jarray;
    }
    return NULL;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clusterface_MainActivity_stopCapture(
        JNIEnv *env,
        jobject /* this */) {
    vd->bcapture = 0;
    if(!vd)
        return;
    v4l2core_capture_stop(vd);
    //MJPGDecoder_destroy();
    SAFEFREE(vd->img.buf);
}

////Face lib test
Face_Handler faceHandler = NULL;


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_clusterface_MainActivity_regFaceInit(JNIEnv *env, jobject instance, jint img_width,
                                                      jint img_height) {

    int ret = 0;
    if(faceHandler)
        return 0;
    ret = initFace();
    if(ret==0)
        return 0;
    LOGW("initFace succeed!");

    char c1[] = "/storage/emulated/0/weights/ar_face_cfg/c1.cfg";
    char w1[] = "/storage/emulated/0/weights/ar_face_cfg/c1.weights";
    char c2[] = "/storage/emulated/0/weights/ar_face_cfg/c2.cfg";
    char w2[] = "/storage/emulated/0/weights/ar_face_cfg/c2.weights";
    char c3[] = "/storage/emulated/0/weights/ar_face_cfg/c3.cfg";
    char w3[] = "/storage/emulated/0/weights/ar_face_cfg/c3.weights";
    char c4[] = "/storage/emulated/0/weights/ar_face_cfg/c4.cfg";
    char w4[] = "/storage/emulated/0/weights/ar_face_cfg/c4.weights";


    faceHandler = createFaceDI(c1,w1,c2,w2,c3,w3,c4,w4,img_width,img_height);
    if(!faceHandler) {
        unInitFace();
        faceHandler = NULL;
        LOGW("createFaceDI failed!");
        return 0;
    }
    LOGW("createFaceDI succeed!");
    return 1;
}


extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_example_clusterface_MainActivity_RegFaceDetect(
        JNIEnv *env,
        jobject instance,jobject bitmap,jboolean breg) {

    if(!faceHandler) {
        return NULL;
    }

    int ret = 0;
    float *pdata = NULL;
    jfloatArray jarray = NULL;

    LOGW("reading bitmap info...");
    AndroidBitmapInfo info;
    //int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0)
    {
        LOGW("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return NULL;
    }
    LOGD("width:%d height:%d stride:%d", info.width, info.height, info.stride);
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        LOGW("Bitmap format is not RGBA_8888!");
        return NULL;
    }
    //
    //read pixels of bitmap into native memory :
    //
    LOGW("reading bitmap pixels...");
    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0)
    {
        LOGW("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    int img_len = info.height * info.stride;
    u_char* img_data = (u_char*) bitmapPixels;

    Fv_Fd_Face * outputData = new Fv_Fd_Face();
    Fv_Fd_Face * tmp = outputData;
    int outputDataLength = 0;
    Fv_Fd_Image fvImg;
    fvImg.channels = 4;
    fvImg.width=info.width;
    fvImg.height = info.height;
    fvImg.imageType = IMG_TYPE_RGB;
    fvImg.imageData = (u_char *)img_data;
    fvImg.imageSize = img_len;

    LOGW("detectFace");
    ret = detectFace(faceHandler,&fvImg,&outputData,&outputDataLength,breg);

    AndroidBitmap_unlockPixels(env, bitmap);

    if(ret!=0) {
        pdata = new float[outputDataLength*270];
        int offset = 0;
        for(int i=0;i<outputDataLength;i++){
            //LOGW("output data %d %d %d %d",outputData[i].rect.x,outputData[i].rect.y,outputData[i].rect.w,outputData[i].rect.h);
            pdata[offset] = (float)outputData[i].rect.x;
            pdata[offset+1] = (float)outputData[i].rect.y;
            pdata[offset+2] = (float)outputData[i].rect.w;
            pdata[offset+3] = (float)outputData[i].rect.h;
            int point_offset = 4;
            for(int j=0;j<5;j++){
                pdata[offset+point_offset] = (float)outputData[i].points[j].x;
                pdata[offset+point_offset+1] = (float)outputData[i].points[j].y;
                point_offset+=2;
            }
            int feature_offset = offset + point_offset;
            for(int j=0;j<256;j++){
                pdata[feature_offset+j] = outputData[i].features[j];
            }
            offset += 270;
        }

        if(outputDataLength>0) {
            jarray = env->NewFloatArray(outputDataLength * 270);
            env->SetFloatArrayRegion(jarray, 0, outputDataLength * 270, (jfloat *) pdata);
        }
        delete(pdata);
    }

    delete tmp;
    if(ret==0)
        return NULL;

    return jarray;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clusterface_MainActivity_RegFaceUninit(JNIEnv *env, jobject instance){

    if(!faceHandler)
        return;
    LOGW("unInitFace");
    unInitFace();
    LOGW("destoryFace");
    destoryFace(faceHandler);
    faceHandler = NULL;
}

#ifdef __cplusplus
}
#endif