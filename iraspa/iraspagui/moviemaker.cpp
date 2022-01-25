/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "moviemaker.h"
#include <QDebug>
#include <iostream>

MovieWriter::MovieWriter(const unsigned int width, const unsigned int height, int fps, LogReporting* logReporter, Format type) :
    _logReporter(logReporter), _type(type), _width(width), _height(height), _fps(fps)
{
}

MovieWriter::~MovieWriter()
{

}

int MovieWriter::initialize(const std::string& filename)
{
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "ffmpeg start initialization");
  }
  qDebug() << "ffmpeg start initialization";

  // register is needed on Ubuntu 18 for snaps
  av_register_all();
  avcodec_register_all();
  qDebug() << "ffmpeg registration";

  _oformat = av_guess_format(nullptr, filename.c_str(), nullptr);
  if (!_oformat)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg could not guess output format");
    }
    return -1;
  }
  switch(_type)
  {
  case Format::h265:
    _oformat->video_codec = AV_CODEC_ID_HEVC;
    break;
  case Format::h264:
    _oformat->video_codec = AV_CODEC_ID_H264;
    break;
  case Format::vp9:
    _oformat->video_codec = AV_CODEC_ID_VP9;
    break;
  case Format::av1:
    _oformat->video_codec = AV_CODEC_ID_AV1;
    break;
  default:
    _oformat->video_codec = AV_CODEC_ID_H264;
    break;
  }
  qDebug() << "ffmpeg form at guessed done";

  int err = avformat_alloc_output_context2(&_ofctx, _oformat, NULL, filename.c_str());
  if (err<0)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create ffmpeg output context");
    }
    return -1;
  }
  qDebug() << "ffmpeg avformat_alloc_output_context2 done";

  _codec = avcodec_find_encoder(_oformat->video_codec);
  if (!_codec)
  {
    avformat_free_context(_ofctx);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create ffmpeg codec");
    }
    return -1;
  }
  qDebug() << "ffmpeg avcodec_find_encoder done";

  // Add a new stream to a media file
  _videoStream = avformat_new_stream(_ofctx, _codec);
  if (!_videoStream)
  {
    avformat_free_context(_ofctx);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create ffmpeg stream");
    }
    return -1;
  }
  qDebug() << "ffmpeg avformat_new_stream done";

  _cctx = avcodec_alloc_context3(_codec);
  if (!_cctx)
  {
    avformat_free_context(_ofctx);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create ffmpeg codec context");
    }
    return -1;
  }
  qDebug() << "ffmpeg avcodec_alloc_context3 done";

  _pkt = av_packet_alloc();
  if (!_pkt)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create ffmpeg packet");
    }
    return -1;
  }
  qDebug() << "ffmpeg av_packet_alloc done";

  AVCodecParameters *codecpar = _videoStream->codecpar;
  if (!codecpar)
  {
      avformat_free_context(_ofctx);
      avcodec_free_context(&_cctx);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to access codecpar of the videostream");
    }
    return -1;
  }
  qDebug() << "ffmpeg codecpar accessed";

  codecpar->codec_id = _oformat->video_codec;
  codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
  codecpar->width = _width;
  codecpar->height = _height;
  codecpar->format = AV_PIX_FMT_YUV420P;

  codecpar->codec_tag = 0;
  switch(_type)
  {
  case Format::h265:
    codecpar->codec_tag = MKTAG('h', 'v', 'c', '1'); // for h265
    codecpar->bit_rate = 5000000;
    break;
  case Format::h264:
    codecpar->bit_rate = 5000000;
    break;
  case Format::vp9:
    codecpar->bit_rate = 5000000;
    break;
  case Format::av1:
    codecpar->bit_rate = 5000000;
    break;
  default:
    codecpar->bit_rate = 5000000;
    break;
  }

  avcodec_parameters_to_context(_cctx, codecpar);

  _cctx->time_base = { 1, _fps };
  _cctx->framerate = {_fps, 1};
  _cctx->max_b_frames = 1;
  _cctx->gop_size = 10;

  if (_videoStream->codecpar->codec_id == AV_CODEC_ID_H264)
  {
    av_opt_set(_cctx, "preset", "slow", 0);
  }
  else if (_videoStream->codecpar->codec_id == AV_CODEC_ID_H265)
  {
    av_opt_set(_cctx, "preset", "slow", 0);
  }


  av_dump_format(_ofctx, 0, filename.c_str(), 1);

  err = avcodec_open2(_cctx, _codec, NULL);
  if (err < 0)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to open codec");
    }
    return -1;
  }
  qDebug() << "ffmpeg codec opened";
  if (_ofctx->oformat->flags & AVFMT_GLOBALHEADER) 
  {
      _cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  avcodec_parameters_from_context(_videoStream->codecpar, _cctx);

  if (!(_oformat->flags & AVFMT_NOFILE))
  {
    err = avio_open(&_ofctx->pb, filename.c_str(), AVIO_FLAG_WRITE);
    if (err < 0)
    {
      avformat_free_context(_ofctx);
      avcodec_free_context(&_cctx);
      av_packet_free(&_pkt);
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to open file");
      }
      return -1;
    }
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "file opened " + QString::fromStdString(filename));
    }
  }
  qDebug() << "ffmpeg avio_open done";

  err = avformat_write_header(_ofctx, NULL);
  if (err < 0)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to write header " + QString::number(err));
    }
    return -1;
  }
  qDebug() << "ffmpeg avformat_write_header done";

  int ret;
  _rgbpic = av_frame_alloc();
  _rgbpic->format = AV_PIX_FMT_BGR0;
  _rgbpic->width = _width;
  _rgbpic->height = _height;
  ret = av_frame_get_buffer(_rgbpic, 0);
  if (ret < 0)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg could not allocate the video frame data");
    }
    return -1;
  }
  qDebug() << "ffmpeg av_frame_alloc done";

  // Allocating memory for each conversion output YUV frame.
  _frame = av_frame_alloc();
  if (!_frame)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg could not allocate a frame");
    }
    return -1;
  }
  qDebug() << "ffmpeg av_frame_alloc done";

  _frame->format = AV_PIX_FMT_YUV420P;
  _frame->width = _width;
  _frame->height = _height;
  ret = av_frame_get_buffer(_frame, 0);
  if (ret < 0)
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    av_frame_free(&_rgbpic);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg could not allocate the video frame data");
    }
    return -1;
  }
  qDebug() << "ffmpeg av_frame_get_buffer done";

  if(!(_swsCtx = sws_getContext(_width, _height, AV_PIX_FMT_BGR0, _width, _height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL)))
  {
    avformat_free_context(_ofctx);
    avcodec_free_context(&_cctx);
    av_packet_free(&_pkt);
    av_frame_free(&_rgbpic);
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg could not allocate a SwsContext");
    }
    return -1;
  }
  qDebug() << "ffmpeg sws_getContext done";

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "ffmpeg initialized");
  }
  qDebug() << "ffmpeg init done";

  return 0;
}

void MovieWriter::encodeFrame(AVFrame *frame)
{
  int ret;

  ret = avcodec_send_frame(_cctx, frame);
  if(ret < 0)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg failed to send frame");
    }
    return;
  }

  while (ret >= 0)
  {
    ret = avcodec_receive_packet(_cctx, _pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0)
    {
      avformat_free_context(_ofctx);
      avcodec_free_context(&_cctx);
      av_packet_free(&_pkt);
      av_frame_free(&_rgbpic);
      av_frame_free(&_frame);
      sws_freeContext(_swsCtx);

      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg error during encoding");
      }
      return;
    }

    av_packet_rescale_ts(_pkt, {1, _fps}, _videoStream->time_base);
    _pkt->stream_index = _videoStream->index;

    av_write_frame(_ofctx, _pkt);
    av_packet_unref(_pkt);
  }
}


void MovieWriter::addFrame(const uint8_t* pixels, size_t iframe)
{
  memcpy(_rgbpic->data[0], pixels, _width * _height * 4);

  sws_scale(_swsCtx, _rgbpic->data, _rgbpic->linesize, 0, _height, _frame->data, _frame->linesize);

  _frame->pts = iframe;

  encodeFrame(_frame);
}

int MovieWriter::finalize()
{
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "ffmpeg finalizing movie");
  }
  
  encodeFrame(NULL);

  av_write_trailer(_ofctx);

  if (!(_oformat->flags & AVFMT_NOFILE))
  {
    int err = avio_close(_ofctx->pb);
    if (err < 0)
    {
      avformat_free_context(_ofctx);
      avcodec_free_context(&_cctx);
      av_packet_free(&_pkt);
      av_frame_free(&_rgbpic);
      av_frame_free(&_frame);
      sws_freeContext(_swsCtx);
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "ffmpeg failed to close file");
      }
    }
  }

  avformat_free_context(_ofctx);
  avcodec_free_context(&_cctx);
  av_packet_free(&_pkt);
  av_frame_free(&_rgbpic);
  av_frame_free(&_frame);
  sws_freeContext(_swsCtx);
  return 0;
}
