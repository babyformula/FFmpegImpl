#include "include/FFmpegDecoder.h"
#include "include/FFmpegH264Encoder.h"
#include <opencv2/opencv.hpp>

YEAH::FFmpegH264Encoder * encoder;
YEAH::FFmpegDecoder * decoder;

int UDPPort;
int HTTPTunnelPort;
pthread_t thread1;
pthread_t thread2;

void * runEncoder(void * encoder)
{
    ((YEAH::FFmpegH264Encoder * ) encoder)->run();
    ((YEAH::FFmpegH264Encoder * ) encoder)->CloseVideo();
    pthread_exit(NULL);
}

void onFrameMain(uint8_t * data, int height, int width)
{
//    cv::Mat ret_img(height, width, CV_8UC3, data);
//    cv::imshow("RGBFrame", ret_img);
//    cv::waitKey();

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

    decoder = new YEAH::FFmpegDecoder("/Users/spectrum/Desktop/0A852916-5E66-5E67-3DFB-365779372B0D20181123_h265.mp4");
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

    decoder->finalize();


    std::cout << "Hello, World!" << std::endl;
    return 0;
}
