#include "faceRecong.hpp"
#include "tensorflow/c/c_api.h"
using namespace cv;
using namespace std;

const string input_layer_1 = "input:0";
const string input_layer_2 = "phase_train:0";
const string output_layer = "embeddings:0";

namespace RESIDEO{

    static void dummy_deallocator(void* data, size_t len, void* arg){
        //free(data);
    }

    tensorflowFacenet::tensorflowFacenet(int inputHeight_, int inputWidth_):inputHeight(inputHeight_),
    inputWidth(inputWidth_){
            depthChannel =3;
            mean_value = 127.5f;
            std_value = 127.5f;
            mapFeatureIndex = 0;
    }

    void tensorflowFacenet::getImageTensor(tensorflow::Tensor &input_tensor, cv::Mat& Image){
        cv::Mat imgResized, sample_float;
        resize(Image, imgResized, Size(160, 160));
        auto input_tensor_mapped = input_tensor.tensor<float, 4>();
        imgResized.convertTo(sample_float, CV_32FC3, 
                            0.007845, 
                            127.5f * 0.007845);
        for (int y = 0; y < inputHeight; ++y) {
            const double* source_row = sample_float.ptr<double>(y);
            for (int x = 0; x < inputWidth; ++x) {
                const double* source_pixel = source_row + (x * depthChannel);
                for (int c = 0; c < depthChannel; ++c) {
                    const double* source_value = source_pixel + (2-c);//RGB->BGR
                    input_tensor_mapped(0, y, x, c) = *source_value;
                }
            }
        }
    }

    void tensorflowFacenet::Recogize(TF_Session * sess, TF_Graph * graph, 
        cv::Mat& Image){
        cv::Mat imgResized;
        cv::resize(Image, imgResized, cv::Size(inputHeight, inputWidth), 0, 0);
        cv::Mat sample_float;
        imgResized.convertTo(sample_float, CV_32FC3, 
                        0.007845, 
                        127.5f * 0.007845);
        /* tensorflow related*/
        TF_Status * s= TF_NewStatus();

        std::vector<TF_Output> input_names;
        std::vector<TF_Tensor*> input_values;

        TF_Operation* input_name = TF_GraphOperationByName(graph, "input");

        input_names.push_back({input_name, 0});

        const int64_t dim[4] = {1,inputHeight,inputWidth,3};

        TF_Tensor* input_tensor = TF_NewTensor(TF_FLOAT,dim,4,sample_float.ptr(), 
                                    sizeof(float)*inputWidth*inputHeight*3, dummy_deallocator,nullptr);

        input_values.push_back(input_tensor);

        /***start**/
        #if 0
        input_name = TF_GraphOperationByName(graph, "phase_train");

        input_names.push_back({input_name, 0});

        const int64_t dimphase[] = {1};
        bool phaseValue[] = {false};

        TF_Tensor *input_tensor_ = TF_NewTensor(TF_BOOL,dimphase, 1, &phaseValue[0],
                                    sizeof(dimphase), dummy_deallocator,nullptr);

        input_values.push_back(input_tensor_);
        #endif
        /***end****/

        std::vector<TF_Output> output_names;

        const char* outOperationName = "output";

        TF_Operation* output_name = TF_GraphOperationByName(graph, outOperationName);
        output_names.push_back({output_name, 0});

        std::vector<TF_Tensor*> output_values(output_names.size(), nullptr);
        
        TF_SessionRun(sess,nullptr,input_names.data(),input_values.data(),input_names.size(),
                output_names.data(),output_values.data(),output_names.size(), nullptr, 0, nullptr, s);
        assert(TF_GetCode(s) == TF_OK);
        /*retrieval the forward results*/
        float *reg_data = (float *)TF_TensorData(output_values[0]);

        std::vector<float> newfeature;
		encodeFeature personFeature;
        for(int ii=0; ii<512; ii++){
            personFeature.featureFace.push_back(reg_data[ii]);
        }
		/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
		if(featureDataBase.size() == 0)
		{
			featureDataBase.insert(std::pair<encodeFeature, std::string>(personFeature, "person_0"));
		}else{
			printf("featureDataBase.size(): %d\n", featureDataBase.size());
			std::map<encodeFeature, std::string>::iterator it = featureDataBase.find(personFeature);
			if(it != featureDataBase.end()){
                std::string name = it->second;
				printf("find this one!, and the person name is : %s\n", name.c_str());
			}else
			{
				featureDataBase.insert(std::pair<encodeFeature, std::string>(personFeature, 
                                                        std::to_string(featureDataBase.size())));
			}	
		}
    }

    double tensorflowFacenet::Recogize(const std::unique_ptr<tensorflow::Session> &session, tensorflow::Tensor& image){

        Tensor phase_train(tensorflow::DT_BOOL, tensorflow::TensorShape());
        phase_train.scalar<bool>()() = false;

        std::vector<Tensor> outputs;
        Status run_status = session->Run({{input_layer_1, image},
                                    {input_layer_2,phase_train }},
                                    {output_layer},
                                    {},
                                    &outputs);
        if(!run_status.ok()){
            LOG(ERROR) << "Running model failed"<<run_status;
            return 0;
        }
        auto outMap = outputs[0].tensor<float, 2>();
        std::vector<float> newfeature;
		encodeFeature personFeature;
        for(int ii=0; ii<512; ii++){
            personFeature.featureFace.push_back(outMap(ii));
        }
		/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
		if(featureDataBase.size() == 0)
		{
			featureDataBase.insert(std::pair<encodeFeature, std::string>(personFeature, "person_0"));
		}else{
			printf("featureDataBase.size(): %d\n", featureDataBase.size());
			std::map<encodeFeature, std::string>::iterator it = featureDataBase.find(personFeature);
			if(it != featureDataBase.end()){
				printf("find this one!, and the person name is : %s\n", it->second);
			}else
			{
				featureDataBase.insert(std::pair<encodeFeature, std::string>(personFeature, 
                                                        std::to_string(featureDataBase.size())));
				mapFeatureIndex++;
			}	
		}
        return 0;
    }

    void tensorflowFacenet::getSession(string graph_path, std::unique_ptr<tensorflow::Session> &session){
        tensorflow::GraphDef graph_def;
        if (!ReadBinaryProto(tensorflow::Env::Default(), graph_path, &graph_def).ok()) {
            LOG(ERROR) << "Read proto";
            return ;
        }
        tensorflow::SessionOptions sess_opt;
        (&session)->reset(tensorflow::NewSession(sess_opt));
        if (!session->Create(graph_def).ok()) {
            LOG(ERROR) << "Create graph";
            return ;
        }
    }

    TF_Session *tensorflowFacenet::load_graph(const char * frozen_fname, TF_Graph** p_graph){
        TF_Status* s = TF_NewStatus();

        TF_Graph* graph = TF_NewGraph();

        std::vector<char> model_buf;

        load_file(frozen_fname,model_buf);

        TF_Buffer graph_def = {model_buf.data(), model_buf.size(), nullptr};

        TF_ImportGraphDefOptions* import_opts = TF_NewImportGraphDefOptions();
        TF_ImportGraphDefOptionsSetPrefix(import_opts, "");
        TF_GraphImportGraphDef(graph, &graph_def, import_opts, s);

        if(TF_GetCode(s) != TF_OK)
        {
            printf("load graph failed!\n Error: %s\n",TF_Message(s));

            return nullptr;
        }

        TF_SessionOptions* sess_opts = TF_NewSessionOptions();
        TF_Session* session = TF_NewSession(graph, sess_opts, s);
        assert(TF_GetCode(s) == TF_OK);

        TF_DeleteStatus(s);

        *p_graph=graph;

        return session;
	}

}//namespace
