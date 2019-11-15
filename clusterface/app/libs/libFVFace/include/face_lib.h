#ifndef _FACE_LIB_H_
#define _FACE_LIB_H_

/*
* Copyright (c) 2018, 上海瞬动科技有限公司合肥分公司研发部
* All rights reserved.
* Create Date:	2018年03月16日
*****************************************************************
模块名          : 瞬动人脸检测库
模块依赖         : FVNet框架模块
文件实现功能     : 人脸检测与识别
作者            : 李乐乐
版本            : 0.0.1
---------------------------------------------------------------------
多线程安全性 : 未测试
---------------------------------------------------------------------
修改记录 : 
日 期				版本		修改人				修改内容
2018/03/15		    0.0.1		李乐乐				初始版本
2018/12/03		    0.0.2		李乐乐				添加识别接口，修改内存释放BUG
2019/02/21		    0.0.3		李乐乐				删除部分内容，上海酷芯专用，ARM64-V8A
*****************************************************************/

//输入图像的类型，涉及到不同类型
#ifndef FV_IMAGE_TYPE
#define FV_IMAGE_TYPE
typedef enum { IMG_TYPE_GRAY,
			   IMG_TYPE_RGB,
			   IMG_TYPE_BGR,
			   IMG_TYPE_YUV,
			   IMG_TYPE_UKNOW } FVIMGTYPE;
#endif

//输入图像数据格式
#ifndef FV_FD_IMAGE
#define FV_FD_IMAGE
typedef struct
{
	int width;				  //图像宽度
	int height;				  //图像高度
	int channels;			  //图像通道
	FVIMGTYPE imageType;	  //图像类型，目前只支持RGB输入！！！！
	int imageSize;			  //图像数据大小
	unsigned char *imageData; //图像数据，此值无效时，应置为固定值（0）
} Fv_Fd_Image;
#endif

//矩形框结构体
#ifndef FV_RECT
#define FV_RECT
typedef struct
{
	int x, y, w, h;
} FvRect;
#endif

//点坐标结构体
#ifndef FV_POINT
#define FV_POINT
typedef struct
{
	int x, y;
} FvPoint;
#endif

//输出人脸数据
#ifndef FV_FACE_DATA
#define FV_FACE_DATA
typedef struct
{
	FvRect rect;		 //人脸框坐标
	FvPoint points[5];		 //五官点坐标
	float features[256]; //如果同时获取人脸特征以做识别用，该数据可用，否则不应尝试获取此数据
} Fv_Fd_Face;
#endif

typedef void *Face_Handler;

//************************************
// 方    法:  initFace()
// 类    型:  public
// 参    数:  void
// 返 回 值:  int              //0：初始化失败;！0：初始化成功
// 功能描述:   初始化网络库，加载相关数据
//************************************
int initFace(void);

//************************************
// 方    法:  createFaceDI()
// 类    型:  public
// 参    数:  cfgPath1            //配置文件路径
// 参    数:  weightsPath1        //权重文件路径
// 参    数:  cfgPath2            //配置文件路径
// 参    数:  weightsPath2        //权重文件路径
// 参    数:  cfgPath3            //配置文件路径
// 参    数:  weightsPath3        //权重文件路径
// 参    数:  cfgPath4            //配置文件路径
// 参    数:  weightsPath4        //权重文件路径
// 参	 数： width				 //图像宽度
// 参    数： height				 //图像高度
// 返 回 值:  Face_Handler   //0：创建句柄失败;！0：句柄创建成功
// 功能描述:  创建网络句柄
//************************************
Face_Handler createFaceDI(const char *cfgPath1, const char *weightsPath1, const char *cfgPath2,
						  const char *weightsPath2, const char *cfgPath3, const char *weightsPath3, const char *g4, const char *w4, int width, int height);

//************************************
// 方    法:  detectFace()
// 类    型:  public
// 参    数:  handler             //句柄
// 参    数:  imageData           //输入图像数据
// 参    数:  outputData          //输出人脸信息，输出个数为outputDataLength
// 参    数：  need_Reg			 //是否需要识别标识，如果该值为1，则outputData中的features有效，否则无效；（如果该值为1，则所有检测到的人脸都会进行特征提取，所需时间较长）
// 返 回 值:  int                 // 0：运行失败;！0：运行成功
// 功能描述:  开始进行人脸检测与识别
//************************************
int detectFace(Face_Handler handler, Fv_Fd_Image *inputData, Fv_Fd_Face **outputData, int *outputDataLength, int need_Reg);

//************************************
// 方    法:  unInitFace()
// 类    型:  public
// 参    数:  void
// 返 回 值:  void
// 功能描述:  反初始化，清理所有资源
//************************************
void unInitFace(void);

//************************************
// 方    法:  destoryFace()
// 类    型:  public
// 参    数:  handler			// 实例句柄
// 返 回 值:  void
// 功能描述:  删除目标实例
//************************************
void destoryFace(Face_Handler handler);

#endif
