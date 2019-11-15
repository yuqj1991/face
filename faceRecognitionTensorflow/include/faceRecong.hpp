#ifndef _RESIDEOFACENETRECO_
#define _RESIDEOFACENETRECO_

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>

#include<math.h>
#include <time.h>
#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgcodecs/imgcodecs.hpp> 
#include "tensorflow/c/c_api.h"

#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/public/session.h"


using namespace std;
using namespace tensorflow;
using tensorflow::Tensor;
using tensorflow::Status;

const float cosValueThresold = 0.65f;
const float euclideanValueThresold = 0.25f;

namespace RESIDEO{
    struct encodeFeature{
        std::vector<float> featureFace;
    };
    struct featureCmp{
        bool operator()(const encodeFeature &leftValue, const encodeFeature &rightValue) const{
            float top =0.0f, bottomLeft=0.0f, bottomRight=0.0f, EuclideanValue = 0.0f;
            assert(leftValue.featureFace.size()==rightValue.featureFace.size());
            assert(leftValue.featureFace.size() == 512);
            for(int ii = 0; ii < 512; ii++){
                top += leftValue.featureFace[ii]*rightValue.featureFace[ii];
                bottomLeft += leftValue.featureFace[ii]*leftValue.featureFace[ii];
                bottomRight += rightValue.featureFace[ii]*rightValue.featureFace[ii];
                EuclideanValue += std::pow((leftValue.featureFace[ii]-rightValue.featureFace[ii]),2);
            }
            float cosValue = (float) (top/(sqrt(bottomLeft)*sqrt(bottomRight)));
            float Euclidean = std::sqrt(EuclideanValue);
            printf("EuclideanValue: %f, cosValue: %f\n", Euclidean, cosValue);
            if(cosValue > cosValueThresold && Euclidean < euclideanValueThresold){
                return false;
            }else{
                if(bottomLeft != bottomRight){
				    return bottomLeft < bottomRight;
                }else{
                    return (bottomLeft + 0.00025) > bottomRight;
			    }
            }
        }
    };
    typedef std::map<encodeFeature, std::string, featureCmp> mapFeature;
    class tensorflowFacenet{
        public:
            explicit tensorflowFacenet(int inputHeight_, int inputWidth_);
            void Recogize(TF_Session * sess, TF_Graph * graph, cv::Mat& Image);
            TF_Session *load_graph(const char* frozen_fname, TF_Graph** p_graph);
            void getImageTensor(tensorflow::Tensor &input_tensor, cv::Mat& Image);
            double Recogize(const std::unique_ptr<tensorflow::Session> &session, tensorflow::Tensor& image);
            void getSession(string graph_path, std::unique_ptr<tensorflow::Session> &session);
        public:
            int inputHeight;
            int inputWidth;
            int depthChannel;
            float mean_value;
            float std_value;
            int mapFeatureIndex;
            mapFeature featureDataBase;

        private:
            inline static int load_file(const std::string & fname, std::vector<char>& buf){
                std::ifstream fs(fname, std::ios::binary | std::ios::in);
                if(!fs.good()){
                    std::cerr<<fname<<" does not exist"<<std::endl;
                    return -1;
                }
                fs.seekg(0, std::ios::end);
                int fsize=fs.tellg();
                fs.seekg(0, std::ios::beg);
                buf.resize(fsize);
                fs.read(buf.data(),fsize);
                fs.close();
                return 0;
            }    
    };
}
#endif
