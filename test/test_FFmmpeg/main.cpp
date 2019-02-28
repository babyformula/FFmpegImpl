#include "FFmpegDecoder.h"
#include "FFmpegH264Encoder.h"
#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <torch/script.h>

#include <opencv2/opencv.hpp>


YEAH::FFmpegH264Encoder * encoder;
YEAH::FFmpegDecoder * decoder;
std::shared_ptr<torch::jit::script::Module> module;

int UDPPort;
int HTTPTunnelPort;
pthread_t thread1;
pthread_t thread2;

void * runEncoder(void * encoder)
{
    ((YEAH::FFmpegH264Encoder * ) encoder)->run();
    pthread_exit(NULL);
}

void onFrameMain(uint8_t * data, int height, int width)
{
    // DEBUG
    cv::Mat ret_img(height*3/2, width, CV_8UC1, data);
    cv::cvtColor(ret_img, ret_img, CV_YUV2BGR_I420);
    //cv::imshow("input_BGRFrame", ret_img);
    
    cv::cvtColor(ret_img, ret_img, cv::COLOR_BGR2RGB);
    
    int h_chop, w_chop;
    h_chop = int(ret_img.rows/2) + 20;
    w_chop = int(ret_img.cols/2) + 20;
    
    cv::Mat ret_slice;
    cv::Rect patch_rect;
    at::Tensor tensor_image, tensor_patch;
    std::vector<at::Tensor> tensors;
    
    {
        patch_rect = cv::Rect(0, 0, w_chop, h_chop);
        ret_img(patch_rect).copyTo(ret_slice);
        
        tensor_patch = torch::from_blob(ret_slice.data, {1, 3, h_chop, w_chop}, at::kByte);
        tensor_patch = tensor_patch.to(at::kFloat);
        tensors.emplace_back(tensor_patch);
    }
    {
        patch_rect = cv::Rect(ret_img.cols-w_chop, 0, w_chop, h_chop);
        ret_img(patch_rect).copyTo(ret_slice);
        
        tensor_patch = torch::from_blob(ret_slice.data, {1, 3, h_chop, w_chop}, at::kByte);
        tensor_patch = tensor_patch.to(at::kFloat);
        tensors.emplace_back(tensor_patch);
    }
    {
        patch_rect = cv::Rect(0, ret_img.rows-h_chop, w_chop, h_chop);
        ret_img(patch_rect).copyTo(ret_slice);
        
        tensor_patch = torch::from_blob(ret_slice.data, {1, 3, h_chop, w_chop}, at::kByte);
        tensor_patch = tensor_patch.to(at::kFloat);
        tensors.emplace_back(tensor_patch);
    }
    {
        patch_rect = cv::Rect(ret_img.cols-w_chop, ret_img.rows-h_chop, w_chop, h_chop);
        ret_img(patch_rect).copyTo(ret_slice);
        
        tensor_patch = torch::from_blob(ret_slice.data, {1, 3, h_chop, w_chop}, at::kByte);
        tensor_patch = tensor_patch.to(at::kFloat);
        tensors.emplace_back(tensor_patch);
    }
    std::cout << tensors.size() << std::endl;
    
    tensor_image = torch::cat(tensors);
    
    std::cout << tensor_image.sizes() << std::endl;
    
    std::vector<torch::jit::IValue> inputs;
//    inputs.push_back(tensor_image);
    inputs.emplace_back(torch::ones({1, 3, 500, 284}));
    at::Tensor output = module->forward(inputs).toTensor();
    
//    cv::Mat output_mat(cv::Size(2*ret_img.rows, 2*ret_img.cols), CV_8UC3, output.data<float>());
    cv::Mat output_mat(cv::Size(2*284, 2*500), CV_8UC3, output.data<float>());
    cv::cvtColor(output_mat, output_mat, cv::COLOR_RGB2BGR);
    
    cv::imshow("out_BGRFrame", output_mat);
    cv::waitKey(20);

    encoder->SendNewFrame(data);
}

int main(int argc, const char * argv[])
{
    if(argc==2)
        decoder = new YEAH::FFmpegDecoder(argv[1]);
    if(argc==3)
        UDPPort = atoi(argv[2]);
    if(argc==4)
        HTTPTunnelPort = atoi(argv[3]);
    
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("/Users/spectrum/Desktop/model.pt");
    
    assert(module != nullptr);
    std::cout << "load model successfully! \n";
    
    decoder = new YEAH::FFmpegDecoder("/Users/spectrum/Downloads/momo/压缩前/momo4.flv");
    decoder->initialize();
    decoder->setOnframeCallbackFunction(onFrameMain);

    // DEBUG
    std::cout
    << "height:     " << decoder->height << "\n"
    << "width:      " << decoder->width << "\n"
    << "frameRate:  " << decoder->frameRate << " [fps]\n"
    << "bitrate:    " << decoder->bitrate << " [bps]\n"
    << "GOP:        " << decoder->GOP << " \n"
    << std::flush;

    encoder = new YEAH::FFmpegH264Encoder();
    encoder->SetupVideo("rtmp://10.37.13.38:19888/live/test",decoder->width,decoder->height,decoder->frameRate,decoder->GOP,decoder->bitrate);
//    server = new YEAH::LiveRTSPServer(encoder, UDPPort, HTTPTunnelPort);
//
//    pthread_attr_t attr1;
//    pthread_attr_init(&attr1);
//    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
//    int rc1 = pthread_create(&thread1, &attr1, runServer, server);
//
//    if (rc1){
//        //exception
//        return -1;
//    }
//
    pthread_attr_t attr2;
    pthread_attr_init(&attr2);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
    int rc2 = pthread_create(&thread2, &attr2, runEncoder, encoder);

    if (rc2){
        //exception
        return -1;
    }

    // Play Media Here
    decoder->playMedia();
    
    encoder->CloseVideo();
    
    decoder->finalize();


    std::cout << "Hello, World!" << std::endl;
    return 0;
}
