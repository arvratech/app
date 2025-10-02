static void _AvVideoSinkStopped(AV_VIDEO_SINK *videoSink)
{
	avVideoOutputStop(videoSink->layerHandle);
	uv_timer_stop(timerVideoSink);
	avCodecStop(videoSink->codec);
	videoSink->state = AV_STATE_IDLE;
}

void avVideoSinkStop(AV_VIDEO_SINK *videoSink)
{
	if(videoSink->state >= AV_STATE_BUSY) {
		_AvVideoSinkStopped(videoSink);
	}
}

void _OnVideoSinkTimer(uv_timer_t *handle)
{
    AV_VIDEO_SINK	*videoSink;
	AV_STREAM_TRACK	*trk;
	AV_CODEC		*codec;
	AV_BUFFER		*buffer, _buffer;
	int64_t		cts, n_pts;
	int		rval, index, val;

    videoSink = (AV_VIDEO_SINK *)handle->data;
	codec = videoSink->codec;
	trk = avCodecOutputTrack(codec);
	buffer = &_buffer;
	rval = -1;	
	while(videoSink->state == AV_STATE_BUSY) {
		index = avCodecDequeueOutputBuffer(codec);
		if(index >= 0) {
			avCodecGetOutputBuffer(codec, index, buffer);
//printf("index=%d frameCount=%d size=%d\n", index, codec->pFrameCount, buffer->size);
			if(buffer->size > 0) {
				if(codec->pFrameCount == 1) {
					videoSink->startTimer = rtcMicroTimer();
					adVideoOutputFirstFrame(videoSink->layerHandle, trk, buffer, &videoSink->_screen);
	//printf("pict: pts=%llu w=%d h=%d fps=%d/%d frame=%d\n", buffer->pts, pict->nWidth, pict->nHeight, pict->nFrameRateNum, pict->nFrameRateDen, pict->nFrameDuration);
				} else {
					adVideoOutputFrame(videoSink->layerHandle, buffer);
				}
				//audioFmt = avStreamOutputGetTrackFormat(stream, stream->audioIndex);
				//if(audioFmt && stream->audioState >= AV_STATE_BUSY) cts = avCodecPresentTimestamp(audioFmt->codec);
				//else	cts = rtcMicroTimer() - videoSink->startTimer;
				cts = rtcMicroTimer() - videoSink->startTimer;
				n_pts = avCodecPresentTimestamp(codec);	// pict->nFrameRateNum, pict->nFrameRateDen;
	//print_status(vd, a_pts, v_pts, a_pts - v_pts);
				val = (n_pts - cts) / 1000;
				if(val < 1) val = 1;
				_VideoSinkTimerStart(val);
				if(buffer->flags & AV_FLAG_END_OF_STREAM) { 
					videoSink->state = AV_STATE_DONE;
				}
				rval = 0;
			} else {
				//if(buffer->flags & AV_FLAG_END_OF_STREAM) { 
				rval = -1;
			}
			avCodecEnqueueOutputBuffer(codec, index); 
//printf("pict: pts=%llu\n", pict->nPts);
			break;
		}
		buffer->data = codecBuf; buffer->size = 0;
		avStreamPull(videoSink, buffer, AV_MEDIA_TYPE_VIDEO);
		if(buffer->status == 0 && buffer->size > 0) {
			buffer->flags = 0;
		} else {
			buffer->size = 0; buffer->flags = AV_FLAG_END_OF_STREAM;
		}
		rval = avCodecPutInputBuffer(codec, buffer);
		if(rval < 0) break;
		codec->dFrameCount++;
	}
	if(rval < 0) {
printf("video stopped...\n");
		_AvVideoSinkStopped(videoSink);
		avStreamOnStopped(videoSink, AV_MEDIA_TYPE_VIDEO);
	}
}

