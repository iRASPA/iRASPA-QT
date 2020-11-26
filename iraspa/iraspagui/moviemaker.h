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

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "logreporting.h"

// based on: https://stackoverflow.com/questions/34511312/how-to-encode-a-video-from-several-images-generated-in-a-c-program-without-wri
//      and: https://github.com/apc-llc/moviemaker-cpp

extern "C"
{
  #include <libswscale/swscale.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/mathematics.h>
  #include <libavformat/avformat.h>
  #include <libavutil/opt.h>
  #include <libavutil/avutil.h>
  #include <libavutil/version.h>
}

class MovieWriter
{
public :
  enum class Type: qint64
  {
    h265 = 0, h264 = 1, vp9 = 2, av1 = 3
  };

  MovieWriter(const unsigned int width, const unsigned int height, int fps, LogReporting *logReporting, Type type);
   ~MovieWriter();
   int initialize(const std::string& filename);
   int finalize();
  void addFrame(const uint8_t* pixels, int iframe);


private:
  LogReporting *_logReporter = nullptr;
  Type _type;

  void encodeFrame(AVFrame *frame);

  const unsigned int _width, _height;
  int _fps;
  SwsContext* _swsCtx;
  AVCodec *_codec;
  AVOutputFormat* _oformat;
  AVStream* _videoStream;
  AVFormatContext* _ofctx;
  AVCodecContext* _cctx;
  AVPacket *_pkt;

  AVFrame *_rgbpic;
  AVFrame *_frame;
};

