#include <string>
#include <queue>
#include <pthread.h>
#include <functional>

extern "C" {

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>

}

namespace YEAH
{
    class FrameStructure {
    public:
        uint8_t * dataPointer;
        int dataSize;
        int frameID;
        ~FrameStructure()
        {
            delete dataPointer;
        }
    };

    class FFmpegH264Encoder
    {
    public:
        FFmpegH264Encoder();
        ~FFmpegH264Encoder();

        void setCallbackFunctionFrameIsReady(std::function<void()> func);

        void SetupVideo(std::string filename, int Width, int Height, int FPS, int GOB, int BitPerSecond);
        void CloseVideo();
        void SetupCodec(const char *filename, int codec_id);
        void CloseCodec();


        void SendNewFrame(uint8_t * RGBFrame);
        void WriteFrame(uint8_t * RGBFrame);
        void WriteVideo(uint8_t * RGBFrame);
        char ReleaseFrame();

        void run();
        char GetFrame(u_int8_t** FrameBuffer, unsigned int *FrameSize);
        
        int  OpenOutFile(const char *szOutFileUrl,char* szFormat);

    private:


        std::queue<uint8_t*> inqueue;
        pthread_mutex_t inqueue_mutex;
        std::queue<FrameStructure *> outqueue;
        pthread_mutex_t outqueue_mutex;


        int m_sws_flags;
        int	m_AVIMOV_FPS;
        int	m_AVIMOV_GOB;
        int	m_AVIMOV_BPS;
        int m_frame_count;
        int	m_AVIMOV_WIDTH;
        int	m_AVIMOV_HEIGHT;
        std::string m_filename;

        double m_video_time;

        AVCodecContext *m_c;
        
        AVStream *m_video_st;
        AVStream *m_audio_st;
        
        AVOutputFormat *m_fmt;
        AVFormatContext *m_oc;
        AVCodec *m_video_codec;
        AVCodec *m_audio_codec;
        AVFrame * m_frame;
        AVPacket pkt;
        
        AVBitStreamFilterContext* m_vbsfc;
        AVBitStreamFilterContext* m_absfc;
        
        SwsContext *sws_ctx;
        uint8_t* picture_buf;
        int bufferSize;
        int framecnt;
        
        int m_videoIndex;
        int m_audioIndex;

        std::function<void()> onFrame;

    };
}
