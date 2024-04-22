#   输出 

```
输出
extern const FFOutputFormat ff_h264_muxer;

```



# 针对 bitstreamfillter


```
FFmpeg\libavcodec/bitstream_filters.c 

static const FFBitStreamFilter *bitstream_filters[] = {

   &ff_h264_metadata_bsf,
    &ff_h264_mp4toannexb_bsf,
    &ff_h264_redundant_pps_bsf,
    &ff_hapqa_extract_bsf,
    &ff_hevc_metadata_bsf,
    &ff_hevc_mp4toannexb_bsf,
}
```