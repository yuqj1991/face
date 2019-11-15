#include <fstream>
#include <utility>
#include <vector>


#include "tensorflow/c/c_api.h"
#include "tensorflow_mtcnn.hpp"
#include "mtcnn.hpp"
#include "comm_lib.hpp"
#include "utils.hpp"

#include "Model.h"
#include "Tensor.h"

#include <unistd.h>
#include "faceRecong.hpp"

using std::string;

using namespace RESIDEO;

int main(int argc, char* argv[]) 
{
	string image = "../data/test.jpg";
	string model_mtcnn = "../models/mtcnn_frozen_model.pb";
	string model_facenet = "../models/facenet.pb";
	string model_facenet_no_phase = "../models/facenet_no.pb";
	string output_fname="../data/new.jpg";

	TF_Session * sess;
	TF_Graph * graph;
	sess=load_graph(model_mtcnn.c_str(),&graph);
	if(sess==nullptr)
		return 1;
	tensorflowFacenet faceReconet(160, 160);
	/********tensorlow/c/api_c使用方法(facenet)**********/
	#if USETENSORFLOW_C_API
	TF_Session * facesess;
	TF_Graph * facegraph;

	facesess = faceReconet.load_graph(model_facenet_no_phase.c_str(),&facegraph);
	if(facesess ==nullptr)
		return 1;
	#else
	/****************************tensorflow 高级框架级api调用方法(facenet)********************/
	tensorflow::Tensor imgTensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1,160,160,3 }));
    unique_ptr<tensorflow::Session> session;
    //facenet model initialize
    faceReconet.getSession(model_facenet, session);
	/*************************************************************************************/
	#endif 

	#if 0
	Model model(model_facenet);
    model.init();
	#endif

	cv::Mat frame = cv::imread(image);
	if(!frame.data)
	{
		std::cerr<<"failed to read image file: "<<image<<std::endl;
		return 1;
	}

	std::vector<face_box> face_info;
	unsigned long start_time=get_cur_time();
	mtcnn_detect(sess,graph,frame,face_info);

	for(unsigned int i=0;i<face_info.size();i++)
	{
		face_box& box=face_info[i];
		cv::Mat RoiImg = frame(cv::Rect(box.x0,box.y0,box.x1 -box.x0,box.y1 -box.y0));
		cv::Mat roi = RoiImg.clone();
		#if USETENSORFLOW_C_API
		faceReconet.Recogize(facesess, facegraph, roi);
		#else
		faceReconet.getImageTensor(imgTensor, roi);
		faceReconet.Recogize(session, imgTensor);
		#endif
		printf("face %d: x0,y0 %2.5f %2.5f  x1,y1 %2.5f  %2.5f conf: %2.5f\n",i,
				box.x0,box.y0,box.x1,box.y1, box.score);
		printf("landmark: ");
		for(unsigned int j=0;j<5;j++)
			printf(" (%2.5f %2.5f)",box.landmark.x[j], box.landmark.y[j]);
		printf("\n");
		cv::rectangle(frame, cv::Point(box.x0, box.y0), cv::Point(box.x1, box.y1), cv::Scalar(0, 255, 0), 1);
		for(int l=0;l<5;l++)
		{
			cv::circle(frame,cv::Point(box.landmark.x[l],box.landmark.y[l]),1,cv::Scalar(0, 0, 255),2);
		}
	}
	unsigned long end_time=get_cur_time();
	cv::imwrite(output_fname,frame);

	std::cout<<"total detected: "<<face_info.size()<<" faces. used "<<double(end_time-start_time)/1000000 <<" s"<<std::endl;
	std::cout<<"boxed faces are in file: "<<output_fname<<std::endl;

	TF_Status* s = TF_NewStatus();

	TF_CloseSession(sess,s);
	TF_DeleteSession(sess,s);
	TF_DeleteGraph(graph);

	#if USETENSORFLOW_C_API
	TF_CloseSession(facesess,s);
	TF_DeleteSession(facesess,s);
	TF_DeleteGraph(facegraph);
	#endif
	TF_DeleteStatus(s);

	return 0;
}







