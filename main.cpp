#include "include/FFmpegDecoder.h"
#include "include/FFmpegH264Encoder.h"
#include <opencv2/opencv.hpp>

YEAH::FFmpegH264Encoder * encoder;
YEAH::FFmpegDecoder * decoder;

int UDPPort;
int HTTPTunnelPort;
pthread_t thread1;
pthread_t thread2;

void onFrame(uint8_t * data)
{
    cv::Mat ret_img(1040, 712, CV_8UC3, data);
    cv::imshow("RGBFrame", ret_img);
    cv::waitKey();

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

    decoder = new YEAH::FFmpegDecoder("/Volumes/G_DRIVE_mobile_SSD_R_Series/lastvideos/0A852916-5E66-5E67-3DFB-365779372B0D20181123_h265.mp4");
    decoder->initialize();
    decoder->setOnframeCallbackFunction(onFrame);

    encoder = new YEAH::FFmpegH264Encoder();
    encoder->SetupVideo("dummy.avi",decoder->width,decoder->height,decoder->frameRate,decoder->GOP,decoder->bitrate);
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
//    pthread_attr_t attr2;
//    pthread_attr_init(&attr2);
//    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
//    int rc2 = pthread_create(&thread2, &attr2, runEncoder, encoder);
//
//    if (rc2){
//        //exception
//        return -1;
//    }

    // Play Media Here
    decoder->playMedia();

    decoder->finalize();


    std::cout << "Hello, World!" << std::endl;
    return 0;
}