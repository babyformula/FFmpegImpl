#include "../include/FFmpegH264Encoder.h"
#include "../include/FFmpegDecoder.h"
#include <opencv2/opencv.hpp>

namespace YEAH
{
    FFmpegH264Encoder::FFmpegH264Encoder()
    {
        pthread_mutex_init(&inqueue_mutex,NULL);
        pthread_mutex_init(&outqueue_mutex,NULL);

        //std::function<void()> callback1 = std::bind(&FFmpegDecoder::onFrame, this);
    }

    FFmpegH264Encoder::~FFmpegH264Encoder()
    {
    }

    void FFmpegH264Encoder::setCallbackFunctionFrameIsReady(std::function<void()> func)
    {
        onFrame = func;
    }


    void FFmpegH264Encoder::SendNewFrame(uint8_t * RGBFrame) {
        pthread_mutex_lock(&inqueue_mutex);

        if(inqueue.size()<30)
        {
            inqueue.push(RGBFrame);
        }
        pthread_mutex_unlock(&inqueue_mutex);
    }

    void FFmpegH264Encoder::run()
    {
        while(true)
        {
            if(!inqueue.empty())
            {
                uint8_t * frame;
                pthread_mutex_lock(&inqueue_mutex);
                frame = inqueue.front();
                inqueue.pop();
                pthread_mutex_unlock(&inqueue_mutex);
                if(frame != NULL)
                {
                    WriteFrame(frame);
                }
            }
        }
    }

    void FFmpegH264Encoder::SetupCodec(const char *filename, int codec_id)
    {
        int ret;
        m_sws_flags = SWS_BICUBIC;
        m_frame_count=0;

        avcodec_register_all();
        av_register_all();

        avformat_alloc_output_context2(&m_oc, NULL, NULL, filename);

        if (!m_oc) {
            avformat_alloc_output_context2(&m_oc, NULL, "mp4", filename);
        }

        if (!m_oc) {
            return;
        }

        m_fmt = m_oc->oformat;
        m_video_st = NULL;
        m_fmt->video_codec = (AVCodecID)codec_id;
        m_fmt->audio_codec = AV_CODEC_ID_NONE;

        AVStream *st;

        m_video_codec = avcodec_find_encoder(m_fmt->video_codec);
        if (!(m_video_codec)) {
            return;
        }

        st = avformat_new_stream(m_oc, m_video_codec);

        if (!st) {
            return;
        }

        st->id = m_oc->nb_streams-1;
        st->time_base.den = m_AVIMOV_FPS;
        st->time_base.num = 1;

        m_c = st->codec;

        m_c->codec_id   = m_fmt->video_codec;
        m_c->codec_type = AVMEDIA_TYPE_VIDEO;
        m_c->bit_rate   = m_AVIMOV_BPS;			//Bits Per Second
        m_c->width      = m_AVIMOV_WIDTH;			//Note Resolution must be a multiple of 2!!
        m_c->height     = m_AVIMOV_HEIGHT;		//Note Resolution must be a multiple of 2!!
        m_c->time_base.den = m_AVIMOV_FPS;        //Frames per second
        m_c->time_base.num = 1;
        m_c->gop_size      = m_AVIMOV_GOB;		// Intra frames per x P frames
        m_c->pix_fmt       = AV_PIX_FMT_YUV420P;//Do not change this, H264 needs YUV format not RGB
        
        m_c->qmin          = 10;
        m_c->qmax          = 51;


        if (m_oc->oformat->flags & AVFMT_GLOBALHEADER)
            m_c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        m_video_st=st;


        AVCodecContext *c = m_video_st->codec;

        ret = avcodec_open2(c, m_video_codec, NULL);
        if (ret < 0) {
            printf("Failed to open encoder! \n");
            return;
        }

        bufferSize = ret;

        av_dump_format(m_oc, 0, filename, 1);

        if (!(m_fmt->flags & AVFMT_NOFILE)) {
            ret = avio_open(&m_oc->pb, filename, AVIO_FLAG_WRITE);
            if (ret < 0) {
                return;
            }
        }

        ret = avformat_write_header(m_oc, NULL);

        if (ret < 0) {
            return;
        }

        sws_ctx = sws_getContext(c->width, c->height, AV_PIX_FMT_YUV420P,
                                 c->width, c->height, AV_PIX_FMT_YUV420P,
                                 SWS_BICUBIC, NULL, NULL, NULL);
        if (!sws_ctx) {
            return;
        }
    }

    void FFmpegH264Encoder::WriteFrame(uint8_t * RGBFrame )
    {
        int picture_size;
        int y_size;
        int framecnt=0;
        
        AVPacket pkt;
        
        AVDictionary *param = 0;
        //H.264
        if(m_c->codec_id == AV_CODEC_ID_H264) {
            av_dict_set(&param, "preset", "slow", 0);
            av_dict_set(&param, "tune", "zerolatency", 0);
            //av_dict_set(&param, "profile", "main", 0);
        }
        //H.265
        if(m_c->codec_id == AV_CODEC_ID_H265){
            av_dict_set(&param, "preset", "ultrafast", 0);
            av_dict_set(&param, "tune", "zero-latency", 0);
        }
        
        //m_video_codec = avcodec_find_encoder(m_c->codec_id);
        if (!m_video_codec){
            printf("Can not find encoder! \n");
            return -1;
        }
        if (avcodec_open2(m_c, m_video_codec, &param) < 0){
            printf("Failed to open encoder! \n");
            return -1;
        }
        
        m_frame = av_frame_alloc();
        picture_size = avpicture_get_size(m_c->pix_fmt, m_c->width, m_c->height);
        avpicture_fill((AVPicture *)m_frame, RGBFrame, m_c->pix_fmt, m_c->width, m_c->height);
        
        //Write File Header
        av_new_packet(&pkt,picture_size);
        
        y_size = m_c->width * m_c->height;
        
        m_frame->width = m_c->width;
        m_frame->height = m_c->height;
        m_frame->format = m_video_st->codec->pix_fmt;
        m_frame->data[0] = RGBFrame;              // Y
        m_frame->data[1] = RGBFrame+ y_size;      // U
        m_frame->data[2] = RGBFrame+ y_size*5/4;  // V
        
        //DEBUG
//        cv::Mat ret_img(m_frame->height*3/2, m_frame->width, CV_8UC1, RGBFrame);
//        cv::cvtColor(ret_img, ret_img, CV_YUV2BGR_I420);
//        cv::imwrite("/Users/spectrum/Desktop/RGBFrame.jpg", ret_img);
        
        //PTS
        //pFrame->pts=i;
        m_frame->pts = m_frame_count*(m_video_st->time_base.den)/((m_video_st->time_base.num)*m_AVIMOV_FPS);
        int got_picture=0;
        //Encode
        int ret = avcodec_encode_video2(m_c, &pkt,m_frame, &got_picture);
        if(ret < 0){
            printf("Failed to encode! \n");
            return -1;
        }
        if (got_picture==1){
            printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
            framecnt++;
            pkt.stream_index = m_video_st->index;
            ret = av_write_frame(m_oc, &pkt);
            av_free_packet(&pkt);
        }

        m_frame_count++;

        //onFrame();
    }
    
    void FFmpegH264Encoder::SetupVideo(std::string filename, int Width, int Height, int FPS, int GOB, int BitPerSecond)
    {
        m_filename = filename;
        m_AVIMOV_WIDTH=Width;	//Movie width
        m_AVIMOV_HEIGHT=Height;	//Movie height
        m_AVIMOV_FPS=FPS;		//Movie frames per second
        m_AVIMOV_GOB=GOB;		//I frames per no of P frames, see note below!
        m_AVIMOV_BPS=BitPerSecond; //Bits per second, if this is too low then movie will become garbled

        SetupCodec(m_filename.c_str(),AV_CODEC_ID_H264);
    }

    void FFmpegH264Encoder::CloseCodec()
    {

        av_write_trailer(m_oc);
        avcodec_close(m_video_st->codec);

        av_freep(&(m_frame->data[0]));
        av_frame_unref(m_frame);
        av_free(m_frame);

        if (!(m_fmt->flags & AVFMT_NOFILE))
            avio_close(m_oc->pb);

        m_oc->pb = NULL;

        avformat_free_context(m_oc);
        sws_freeContext(sws_ctx);

    }

    void FFmpegH264Encoder::CloseVideo()
    {
        CloseCodec();
    }

    char FFmpegH264Encoder::GetFrame(u_int8_t** FrameBuffer, unsigned int *FrameSize)
    {
        if(!outqueue.empty())
        {
            FrameStructure * frame;
            frame  = outqueue.front();
            *FrameBuffer = (uint8_t*)frame->dataPointer;
            *FrameSize = frame->dataSize;
            return 1;
        }
        else
        {
            *FrameBuffer = 0;
            *FrameSize = 0;
            return 0;
        }
    }

    char FFmpegH264Encoder::ReleaseFrame()
    {
        pthread_mutex_lock(&outqueue_mutex);
        if(!outqueue.empty())
        {
            FrameStructure * frame = outqueue.front();
            outqueue.pop();
            delete frame;
        }
        pthread_mutex_unlock(&outqueue_mutex);
        return 1;
    }
}
