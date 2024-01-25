#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
static char *InputFileName  = NULL;
static char *OutputFileName = NULL;
static FILE *pInput_File     = NULL;
static FILE *pOutput_File    = NULL;

static int frameWidth    = 0;
static int frameHeight   = 0;
static int bitRate       = 0;
static int frameToEncode = 0;

static enum AVCodecID codec_id = AV_CODEC_ID_NONE;
static char *strcodec = NULL;

void parse_argv(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++){
        printf("argv %d : %s\n", i, argv[i]);
        if(!strcmp(argv[i], "-i")) InputFileName  = argv[++i];
        if(!strcmp(argv[i], "-o")) OutputFileName = argv[++i];
        if(!strcmp(argv[i], "-codec")){
            i++;
            if(!strcmp(argv[i], "264"))   { codec_id   = AV_CODEC_ID_H264; strcodec = "264";}
            if(!strcmp(argv[i], "265"))   { codec_id   = AV_CODEC_ID_H265; strcodec = "265";}
            if(!strcmp(argv[i], "mpeg1")) { codec_id   = AV_CODEC_ID_MPEG1VIDEO; strcodec = "mpeg1";}
            if(!strcmp(argv[i], "mpeg2")) { codec_id   = AV_CODEC_ID_MPEG1VIDEO; strcodec = "mpeg2";}
        }
        if(!strcmp(argv[i], "-w")) frameWidth  = atoi(argv[++i]);
        if(!strcmp(argv[i], "-h")) frameHeight = atoi(argv[++i]);
    }


    pInput_File = fopen(InputFileName, "rb+");
    if(!pInput_File){
        fprintf(stderr, "open input file fail\n"); 
        return;
    }

    pOutput_File = fopen(OutputFileName, "wb+");
    if(!pOutput_File){
        fprintf(stderr, "open output file fail\n");
        return;
    }

    printf("input %s output %s codec %s\n", InputFileName, OutputFileName, strcodec);

    return;
}

int  main(int argc, char **argv)
{
    int ret = 0;
    int i, x, y;
    int got_output;
    AVCodec *codec = NULL;
    AVCodecContext *codecCtx = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt;

    if(argc < 5){
        fprintf(stderr, "Usage:%s -i <inputfilename> -o <outputfilename> [-codec (264|265|mpeg1|mpeg2)]\n", argv[0]); 
        exit(1);
    }
    AVFormatContext* push_format_context_ptr = avformat_alloc_context();
    const char* outFileName = "udp://@239.255.255.250:54546?pkt_size=1316"; //nginx 流服务器地址
    ret = avformat_alloc_output_context2(&push_format_context_ptr, NULL,  "mpegts", outFileName);
    AVStream* pOutStream = NULL;

    printf("ret = %u\n", ret);
    pOutStream = avformat_new_stream(push_format_context_ptr, NULL); //分配流空间
    if (push_format_context_ptr->oformat->flags & AVFMT_GLOBALHEADER)
    {
        push_format_context_ptr->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    //A
    //parse argument
    parse_argv(argc, argv);
    avformat_network_init();
   // av_register_all();
    //avcodec_register_all();

    codec = avcodec_find_encoder(codec_id);
    if(!codec){
        fprintf(stderr, "Could not find the encoder\n");
        return -1;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx){
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    codecCtx->bit_rate = 400000;
    codecCtx->width    = frameWidth;
    codecCtx->height   = frameHeight;
    codecCtx->time_base = (AVRational){1, 25};
    codecCtx->gop_size  = 10;
    codecCtx->max_b_frames = 1;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    //av_opt_set(codecCtx->priv_data, "preset", "slow", 0);

    //open the encoder
    if(avcodec_open2(codecCtx, codec, NULL) < 0){
        fprintf(stderr, "Open encoder fail\n");
        return -1;
    }
    ret = avcodec_parameters_from_context(push_format_context_ptr->streams[0]->codecpar, codecCtx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", ret);
        return ret;
    }
    // pkt_size = 1316 // ts page_size
    AVDictionary* options = NULL;
    ret = av_dict_set(&options, "pkt_size", "1316", AVIO_FLAG_WRITE);
    const AVDictionaryEntry* e = NULL;
    /*while ((e = av_dict_iterate(options, e))) {
        printf("[key = %s][value = %s]\n", e->key, e->value);
    }*/
    while (e = av_dict_get(options, "", e, AV_DICT_IGNORE_SUFFIX)) {
        printf("[key = %s][value = %s]\n", e->key, e->value);
    }
    //****分配推流io上下文****//
    ret = avio_open(&push_format_context_ptr->pb, outFileName, AVIO_FLAG_WRITE, NULL, &options);
    if (ret < 0) {
        printf("Could not open output file '%s'\n", outFileName);
        return -1;
    }
    av_dict_free(&options);
    options = NULL;
    ret = avformat_write_header(push_format_context_ptr, NULL); //写入头
    // pOutFormatContext->streams[0]->time_base.num *= 10;
    if (ret < 0) {
        printf("Error occurred when opening output file\n");
        return -1;
    }

    ////////////

    //allocate AVFrame structure
    frame = av_frame_alloc();
    if(!frame){
        fprintf(stderr, "Could not allocate wideo frame\n"); 
        return -1;
    }
    frame->format  = codecCtx->pix_fmt;
    frame->width   = codecCtx->width;
    frame->height  = codecCtx->height;

    //allocate AVFrame data 
    ret = av_image_alloc(frame->data, frame->linesize, codecCtx->width, codecCtx->height,
                         codecCtx->pix_fmt, 32);
    if(ret < 0){
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        return -1;
    }

    pkt = av_packet_alloc();
    
    int count = 0;
    while(1){
        //init AVPacket
       
        pkt->data = NULL;
        pkt->size = 0;

        fflush(stdout);
        //prepare a dummy image
        /*Y*/
        /*
        for(y = 0; y < codecCtx->height; y++){
            for(x = 0; x < codecCtx->width; x++){
                frame->data[0][y*frame->linesize[0] + x] = x + y + i*3;
            }
        }
        */

        /*Cb and Cr*/
        /*
        for(y = 0; y < codecCtx->height/2; y++){
            for(x = 0; x < codecCtx->width/2; x++){
                frame->data[1][y*frame->linesize[1] + x] = 128 + y + i*2;
                frame->data[2][y*frame->linesize[2] + x] = 64 + x + i*5;
            }
        }
        */
        fread(frame->data[0], 1, codecCtx->height * codecCtx->width, pInput_File);
        fread(frame->data[1], 1, (codecCtx->height/2) * (codecCtx->width/2), pInput_File);
        fread(frame->data[2], 1, (codecCtx->height/2) * (codecCtx->width/2), pInput_File);


        frame->pts = ++count;

        //encode the image
       // ret = avcodec_encode_video2(codecCtx, &pkt, frame, &got_output);
        ret = avcodec_send_frame(codecCtx, frame);
        ret = avcodec_receive_packet(codecCtx, pkt);
        if(ret < 0){
            fprintf(stderr, "Error encoding frame\n");
            continue;
            //return -1;
        }
        printf("frame push [%s][%u]--> \n", outFileName, count);
        // 4. 将编码后的packet写入输出媒体文件
        ret = av_interleaved_write_frame(push_format_context_ptr, pkt);
        // av_packet_unref(&encodePacket);

       // if(got_output){
           // printf("Write frame %3d (size=%5d) \n", i, pkt->size);
           // fwrite(pkt->data, 1, pkt->size, pOutput_File);
           // av_packet_unref(pkt);
        //}
    }

    //get the delayed frames
    for(got_output = 1; got_output;){
        fflush(stdout);

        // ret = avcodec_encode_video2(pEncodeContext,&encodePacket,pInFrame,&gotPicture);
        ret = avcodec_send_frame(codecCtx, frame);
        ret = avcodec_receive_packet(codecCtx, pkt);
       // ret = avcodec_encode_video2(codecCtx, &pkt, NULL, &got_output);
        if(ret < 0){
            fprintf(stderr, "Error encoding frame\n");
            return -1;
        }

       // if(got_output){
            printf("Write frame %3d (size=%5d) \n", i, pkt->size);
            fwrite(pkt->data, 1, pkt->size, pOutput_File);
            av_packet_unref(&pkt);
        //}
    }

    return 0;
}
