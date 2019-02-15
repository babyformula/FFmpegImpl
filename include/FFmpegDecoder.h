#include <iostream>
#include <string>
#include <pthread.h>
#include <functional>
#include <unistd.h>

extern "C" {
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>
}

namespace YEAH
{
    class FFmpegDecoder
    {
    public:
        FFmpegDecoder(std::string);

        ~FFmpegDecoder();

        void initialize();

        void playMedia();

        void finalize();

        void setOnframeCallbackFunction(std::function<void(uint8_t *, int, int)> func);

        int width;

        int height;

        int GOP;

        int frameRate;

        int bitrate;
        
        int FPS;

        std::function<void(uint8_t *, int, int)> onFrame;

    private:

        std::string path;

        AVCodecContext  *pCodecCtx;
        
        AVCodecContext  *vCodecCtx;
        AVCodecContext  *aCodecCtx;

        AVFormatContext *pFormatCtx;

        AVFrame *pFrameRGB;

        struct SwsContext * img_convert_ctx;

        int videoStreamIdx;
        int audioStreamIdx;

        int videoCodecID;
        int audioCodecID;
        
        int channels;
        int sampleRate;
        
        bool isHaveVideo;
        bool isHaveAudio;

    };
}


