#include "../include/FFmpegDecoder.h"

namespace YEAH
{
    FFmpegDecoder::FFmpegDecoder(std::string path)
    {
        this->path = path;
        
        isHaveAudio = false;
        isHaveVideo = false;
        
        videoStreamIdx = -1;
        audioStreamIdx = -1;
        
        videoCodecID = -1;
        audioCodecID = -1;
    }


    void FFmpegDecoder::initialize()
    {
        int result = 1;

        // Intialize FFmpeg enviroment
        av_register_all();
        avdevice_register_all();

        avformat_network_init();

        const char  *filenameSrc = path.c_str();

        pFormatCtx = avformat_alloc_context();

        AVCodec * pCodec;
        
        if (pFormatCtx == NULL)
        {
            result = 222;
            return;
        }

        if(avformat_open_input(&pFormatCtx,filenameSrc,NULL,NULL) != 0)
        {
            //exception
            result = 201;
            return;
        }

        if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
        {
            //exception
            result = 202;
            return;
        }

        av_dump_format(pFormatCtx, 0, filenameSrc, 0);

        for(int i=0; i < pFormatCtx->nb_streams; i++)
        {
            if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIdx == -1)
            {
                videoStreamIdx = i;
                
                AVStream *stream = pFormatCtx->streams[i];
                videoCodecID = stream->codecpar->codec_id;
                AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
                if (dec) {
                    vCodecCtx = avcodec_alloc_context3(dec);
                    if (vCodecCtx) {
                        avcodec_parameters_to_context(vCodecCtx, stream->codecpar);
                    }
                }
                width = pFormatCtx->streams[i]->codec->width;
                height = pFormatCtx->streams[i]->codec->height;
                frameRate = 0.0;
                if(pFormatCtx->streams[i]->r_frame_rate.den > 0)
                {
                    frameRate = pFormatCtx->streams[i]->r_frame_rate.num /(double)pFormatCtx->streams[i]->r_frame_rate.den;
                }
                else
                {
                    frameRate = 1000;
                }
                
                if(frameRate > 100)
                {
                    FPS = 15;
                }
                else
                {
                    FPS = (int)frameRate;
                }
                isHaveVideo = true;
                
                bitrate = pFormatCtx->streams[i]->codec->bit_rate;
                GOP = pFormatCtx->streams[i]->codec->gop_size;
            }
            else if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStreamIdx == -1)
            {
                audioStreamIdx = i;
                AVStream *stream = pFormatCtx->streams[i];
                AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
                audioCodecID = stream->codecpar->codec_id;
                if (dec) {
                    aCodecCtx = avcodec_alloc_context3(dec);
                    if (aCodecCtx) {
                        avcodec_parameters_to_context(aCodecCtx, stream->codecpar);
                    }
                }
                channels = stream->codecpar->channels;
                sampleRate = stream->codecpar->sample_rate;
                
                isHaveAudio = true;
            }
//            AVStream *st = pFormatCtx->streams[i];
//            enum AVMediaType type = st->codec->codec_type;
//            if (videoStreamIdx == -1)
//                if (avformat_match_stream_specifier(pFormatCtx, st, "vst") > 0)
//                    videoStreamIdx = i;
        }

//        videoStreamIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO,videoStreamIdx, -1, NULL, 0);

        if(videoStreamIdx == -1)
        {
            //exception
            return;
        }

        pCodecCtx = pFormatCtx->streams[videoStreamIdx]->codec;

        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL)
        {
            //exception
            return;
        }

        pCodecCtx->codec_id = pCodec->id;
        pCodecCtx->workaround_bugs   = 1;

        if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0)
        {
            //exception
            return;
        }

        pFrameRGB = av_frame_alloc();
        AVPixelFormat  pFormat = AV_PIX_FMT_YUV420P;
        uint8_t *fbuffer;
        int numBytes;
        numBytes = avpicture_get_size(pFormat,pCodecCtx->width,pCodecCtx->height) ; //AV_PIX_FMT_RGB24
        fbuffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
        avpicture_fill((AVPicture *) pFrameRGB,fbuffer,pFormat,pCodecCtx->width,pCodecCtx->height);

        img_convert_ctx = sws_getCachedContext(NULL,pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,   pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL,NULL);

//        height = pCodecCtx->height;
//        width =  pCodecCtx->width;
//        bitrate =pCodecCtx->bit_rate;
//        GOP = pCodecCtx->gop_size;
//        frameRate = (int )pFormatCtx->streams[videoStreamIdx]->avg_frame_rate.num/pFormatCtx->streams[videoStreamIdx]->avg_frame_rate.den;

    }

    void FFmpegDecoder::setOnframeCallbackFunction(std::function<void(uint8_t *, int, int)> func)
    {
        onFrame = func;
    }


    void FFmpegDecoder::playMedia()
    {
        AVPacket packet;
        AVFrame * pFrame;
        while((av_read_frame(pFormatCtx,&packet)>=0))
        {
            if(packet.buf != NULL & packet.stream_index == videoStreamIdx)
            {
                pFrame = av_frame_alloc();
                int frameFinished;
                int decode_ret = avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);
                av_free_packet(&packet);
                if(frameFinished)
                {
                    sws_scale(img_convert_ctx, ((AVPicture*)pFrame)->data, ((AVPicture*)pFrame)->linesize, 0, pCodecCtx->height, ((AVPicture *)pFrameRGB)->data, ((AVPicture *)pFrameRGB)->linesize);
                    onFrame(((AVPicture *)pFrameRGB)->data[0], pCodecCtx->height, pCodecCtx->width);
                }
                av_frame_unref(pFrame);
                av_free(pFrame);
            }
//            else if(packet.buf != NULL & packet.stream_index == audioStreamIdx)
//            {
//                if(m_nAPts == -1)
//                m_nAPts = pkt->pts;
//                m_nADuration = (pkt->pts - m_nAPts)/in_stream->time_base.den * 1000;
//                bIsVideo = false;
//                break;
//            }
            
            usleep(((double)(1.0/frameRate))*1000000);

        }
        av_free_packet(&packet);


    }

    void FFmpegDecoder::finalize()
    {
        sws_freeContext(img_convert_ctx);
//        if (pFrameRGB->data[0] != NULL)
//        {
//            std::cout >> &(pFrameRGB->data[0]) >> std::endl;
//            av_freep(&(pFrameRGB->data[0]));
//        }
        av_frame_unref(pFrameRGB);
        av_free(pFrameRGB);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);
    }

}
