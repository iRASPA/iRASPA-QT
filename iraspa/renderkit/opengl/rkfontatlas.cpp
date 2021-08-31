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

#include "rkfontatlas.h"
#include "foundationkit.h"
const int scaler = 16;

RKFontAtlas::RKFontAtlas(QString fontName, int texture_size): width(texture_size), height(texture_size), textureData(texture_size * texture_size), _pdata( 4 * texture_size * texture_size, 0 ), characters()
{
  QFont font;
  font.fromString(fontName);
  QRawFont rawFont = QRawFont:: fromFont(font);

  renderSignedDistanceFont(rawFont, texture_size);

  for(size_t i=0; i<characters.size(); i++)
  {
    int index = characters[i].ID;
    characterIndex[index] = int(i);
  }
}

RKFontAtlas::~RKFontAtlas()
{
}

QImage RKFontAtlas::mapForGlyph(QRawFont &rawFont, quint32 glyph_index)
{
  QRectF rect = rawFont.boundingRect(glyph_index);
  int glyph_width = rint(rect.width());
  int glyph_height = rint(rect.height());

  if (glyph_width <= 0 || glyph_height <= 0)
      return QImage();

  QPainterPath path =	rawFont.pathForGlyph(glyph_index);
  path.translate(-path.boundingRect().topLeft());
  path.setFillRule(Qt::WindingFill);
  QImage im(glyph_width, glyph_height, QImage::Format_Mono);
  im.fill(Qt::transparent);
  QPainter p(&im);

  p.drawPath(path);
  p.end();
  return im;
}


bool RKFontAtlas::renderSignedDistanceFont(QRawFont &rawFont, int texture_size)
{
  int max_unicode_char= 255;

  //	Try all characters up to 255 values (it will auto-skip any without glyphs)
  std::vector< int > render_list;
  for( int char_idx = 0; char_idx <= max_unicode_char; ++char_idx )
  {
    render_list.push_back( char_idx );
  }

  characters.clear();
  int sz = fontSizeForTextureSize(rawFont, texture_size, render_list, characters);
  rawFont.setPixelSize(sz*scaler);

  // render all the glyphs individually
  int packed_glyph_index = 0;
  int tin = clock();
  for( unsigned int char_index = 0; char_index < render_list.size(); ++char_index )
  {
    QVector<quint32> glyph_index2 = rawFont.glyphIndexesForString(QString(QChar(render_list[char_index])));
    int glyph_index = glyph_index2[0];
    if( glyph_index )
    {
      QImage image = mapForGlyph(rawFont, glyph_index);

      int w = image.width() ;
      int h = image.height();
      int p = image.bytesPerLine();

      //	oversize the holding buffer so we can smooth it
      int sw = w + scaler * 4;
      int sh = h + scaler * 4;
      unsigned char *smooth_buf = new unsigned char[sw*sh];
      for( int i = 0; i < sw*sh; ++i )
      {
        smooth_buf[i] = 0;
      }

      //	copy the glyph into the buffer to be smoothed
      unsigned char *buf = image.bits();
      for( int j = 0; j < h; ++j )
      {
        for( int i = 0; i < w; ++i )
        {
          smooth_buf[scaler*2+i+(j+scaler*2)*sw] = 255 * ((buf[j*p+(i>>3)] >> (7 - (i & 7))) & 1);
        }
      }
      //	do the SDF
      int sdfw = characters[packed_glyph_index].width;
      int sdfx = characters[packed_glyph_index].x;
      int sdfh = characters[packed_glyph_index].height;
      int sdfy = characters[packed_glyph_index].y;
      for( int j = 0; j < sdfh; ++j )
      {
        for( int i = 0; i < sdfw; ++i )
        {
          int pd_idx = (i+sdfx+(j+sdfy)*texture_size) * 4;
          _pdata[pd_idx] =
             //get_SDF
            get_SDF_radial( smooth_buf, sw, sh, i*scaler + (scaler >>1), j*scaler + (scaler >>1), 2*scaler );
            _pdata[pd_idx+1] = _pdata[pd_idx];
            _pdata[pd_idx+2] = _pdata[pd_idx];
            _pdata[pd_idx+3] = _pdata[pd_idx];
          textureData[(i+sdfx+(j+sdfy)*texture_size)] = _pdata[pd_idx];
        }
      }
      packed_glyph_index++;

      delete [] smooth_buf;
    }
  }
  tin = clock() - tin;

  return true;
}

int RKFontAtlas::fontSizeForTextureSize(QRawFont &rawFont, int texture_size, const std::vector< int > &render_list, std::vector< FontCharacter > &c)
{
  //	initial guess for the size of the Signed Distance Field font
  //	(intentionally low, the first trial will be at sz*2, so 8x8)
  int sz = 4;
  bool keep_going = true;
  while( keep_going )
  {
    sz <<= 1;
    keep_going = gen_pack_list(rawFont, sz, texture_size, render_list, c );
  }
  int sz_step = sz >> 2;
  while( sz_step )
  {
    if( keep_going )
    {
      sz += sz_step;
    } else
    {
      sz -= sz_step;
    }
    sz_step >>= 1;
    keep_going = gen_pack_list(rawFont, sz, texture_size, render_list, c );
  }
  //	just in case
  while( (!keep_going) && (sz > 1) )
  {
    --sz;
    keep_going = gen_pack_list(rawFont, sz, texture_size, render_list, c );
  }
  qDebug() <<  "Result number of pixels: " + QString::number(sz);

  if( !keep_going )
  {
    // The data will not fit in a texture texture_size^2
    return -1;
  }

  return sz;
}

bool RKFontAtlas::gen_pack_list(QRawFont &rawFont, int pixel_size, int pack_tex_size, const std::vector< int > &render_list, std::vector< FontCharacter > &packed_glyphs)
{
  packed_glyphs.clear();
  rawFont.setPixelSize(pixel_size*scaler);

  std::vector< int > rectangle_info;
  std::vector< std::vector<int> > packed_glyph_info;
  for( unsigned int char_index = 0; char_index < render_list.size(); ++char_index )
  {
    QVector<quint32> glyph_index2 = rawFont.glyphIndexesForString(QString(QChar(render_list[char_index])));
    int glyph_index = glyph_index2[0];
    if( glyph_index )
    {
      QRectF rect = rawFont.boundingRect(glyph_index);
      int w = rint(rect.width());
      int h = rint(rect.height());

      FontCharacter add_me;

      // oversize the holding buffer so I can smooth it!
      int sw = w + scaler * 4;
      int sh = h + scaler * 4;
      //	do the SDF
      int sdfw = sw / scaler;
      int sdfh = sh / scaler;
      rectangle_info.push_back( sdfw );
      rectangle_info.push_back( sdfh );
      //	add in the data I already know
      add_me.ID = render_list[char_index];
      add_me.width = sdfw;
      add_me.height = sdfh;
      //	these need to be filled in later (after packing)
      add_me.x = -1;
      add_me.y = -1;
      //	these need scaling...
      add_me.xoff = rint(rect.x());
      add_me.yoff = -rint(rect.y());


      const QList<QPointF> pointList = rawFont.advancesForGlyphIndexes(glyph_index2);
      add_me.xadv = rint(pointList[0].x());
      add_me.yadv = rint(pointList[0].y());
      //	so scale them (the 1.5's have to do with the padding
      //	border and the sampling locations for the SDF)
      add_me.xoff = add_me.xoff / scaler - 1.5;
      add_me.yoff = add_me.yoff / scaler + 1.5;
      add_me.xadv = add_me.xadv / scaler;
      //	add it to my list
      packed_glyphs.push_back( add_me );
    }
  }

  const bool dont_allow_rotation = false;
  BinPacker bp;
  bp.Pack( rectangle_info, packed_glyph_info, pack_tex_size, dont_allow_rotation );
  //	populate the actual coordinates
  if( packed_glyph_info.size() == 1 )
  {
    //	it all fit into one!
    unsigned int lim = uint(packed_glyph_info[0].size());
    for( unsigned int i = 0; i < lim; i += 4 )
    {
      //	index, x, y, rotated
      unsigned int idx = packed_glyph_info[0][i+0];
      packed_glyphs[idx].x = packed_glyph_info[0][i+1];
      packed_glyphs[idx].y = packed_glyph_info[0][i+2];
    }
    return true;
  }
  return false;
}

int RKFontAtlas::save_png_SDFont(
    const char* orig_filename,
    const char* font_name,
    int img_width, int img_height,
    const std::vector< unsigned char > &img_data,
    const std::vector< FontCharacter > &packed_glyphs )
{
  //	save my image
  size_t fn_size = strlen( orig_filename ) + 100;
  char *fn = new char[ fn_size ];
  sprintf( fn, "%s_sdf.png", orig_filename );
  printf( "'%s'\n", fn );
  LodePNG::Encoder encoder;
  encoder.addText("Comment", "Signed Distance Font: lonesock tools");
  encoder.getSettings().zlibsettings.windowSize = 512; //	faster, not much worse compression
  std::vector<unsigned char> buffer;
  int tin = clock();
  encoder.encode( buffer, img_data.empty() ? 0 : &img_data[0], img_width, img_height );
  LodePNG::saveFile( buffer, fn );
  tin = clock() - tin;

  //	now save the acompanying info
  sprintf( fn, "%s_sdf.txt", orig_filename );
  FILE *fp = fopen( fn, "w" );
  if( fp )
  {
    fprintf( fp, "info face=\"%s\"\n",
        font_name  );
    fprintf( fp, "chars count=%i\n", int(packed_glyphs.size()) );
    for( unsigned int i = 0; i < packed_glyphs.size(); ++i )
    {
      fprintf( fp, "char id=%-6ix=%-6iy=%-6iwidth=%-6iheight=%-6i",
        packed_glyphs[i].ID,
        packed_glyphs[i].x,
        packed_glyphs[i].y,
        packed_glyphs[i].width,
        packed_glyphs[i].height
        );
      fprintf( fp, "xoffset=%-10.3fyoffset=%-10.3fxadvance=%-10.3f",
        packed_glyphs[i].xoff,
        packed_glyphs[i].yoff,
        packed_glyphs[i].xadv
        );
      fprintf( fp, "  page=0  chnl=0\n" );
    }
    fclose( fp );
  }
  delete [] fn;
  return tin;
}

unsigned char RKFontAtlas::get_SDF_radial(
    unsigned char *fontmap,
    int w, int h,
    int x, int y,
    int max_radius )
{
  //	hideous brute force method
  float d2 = max_radius*max_radius+1.0;
  unsigned char v = fontmap[x+y*w];
  for( int radius = 1; (radius <= max_radius) && (radius*radius < d2); ++radius )
  {
    int line, lo, hi;
    //	north
    line = y - radius;
    if( (line >= 0) && (line < h) )
    {
      lo = x - radius;
      hi = x + radius;
      if( lo < 0 ) { lo = 0; }
      if( hi >= w ) { hi = w-1; }
      int idx = line * w + lo;
      for( int i = lo; i <= hi; ++i )
      {
        //	check this pixel
        if( fontmap[idx] != v )
        {
          float nx = i - x;
          float ny = line - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //	move on
        ++idx;
      }
    }
    //	south
    line = y + radius;
    if( (line >= 0) && (line < h) )
    {
      lo = x - radius;
      hi = x + radius;
      if( lo < 0 ) { lo = 0; }
      if( hi >= w ) { hi = w-1; }
      int idx = line * w + lo;
      for( int i = lo; i <= hi; ++i )
      {
        //	check this pixel
        if( fontmap[idx] != v )
        {
          float nx = i - x;
          float ny = line - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //	move on
        ++idx;
      }
    }
    //	west
    line = x - radius;
    if( (line >= 0) && (line < w) )
    {
      lo = y - radius + 1;
      hi = y + radius - 1;
      if( lo < 0 ) { lo = 0; }
      if( hi >= h ) { hi = h-1; }
      int idx = lo * w + line;
      for( int i = lo; i <= hi; ++i )
      {
        //	check this pixel
        if( fontmap[idx] != v )
        {
          float nx = line - x;
          float ny = i - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //	move on
        idx += w;
      }
    }
    //	east
    line = x + radius;
    if( (line >= 0) && (line < w) )
    {
      lo = y - radius + 1;
      hi = y + radius - 1;
      if( lo < 0 ) { lo = 0; }
      if( hi >= h ) { hi = h-1; }
      int idx = lo * w + line;
      for( int i = lo; i <= hi; ++i )
      {
        //	check this pixel
        if( fontmap[idx] != v )
        {
          float nx = line - x;
          float ny = i - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //	move on
        idx += w;
      }
    }
  }
  d2 = sqrtf( d2 );
  if( v==0 )
  {
    d2 = -d2;
  }
  d2 *= 127.5 / max_radius;
  d2 += 127.5;
  if( d2 < 0.0 ) d2 = 0.0;
  if( d2 > 255.0 ) d2 = 255.0;
  return (unsigned char)(d2 + 0.5);
}

std::vector<RKInPerInstanceAttributesText> RKFontAtlas::buildMeshWithString(float4 position, float4 scale, QString text, RKTextAlignment alignment)
{
    float x = 0.0;
    float y = 0.0;
    float sx = 1.0;
    float sy = 1.0;

    std::vector<RKInPerInstanceAttributesText> subdata{};
    QRectF rect = QRectF();
    for (int i = 0 ; i < text.size(); i++)
    {
      int id = characterIndex[text[i].unicode()];
      if(id >= 0 && id<=255)
      {
        float x2 = x + characters[id].xoff * sx;
        float y2 = -y - characters[id].yoff * sy;
        float w = characters[id].width * sx;
        float h = characters[id].height * sy;

        x += characters[id].xadv * sx;
        y += characters[id].yadv * sy;

        float4 vertex = float4(x2,y2,w,h);
        rect=rect.united(QRectF(x2,y2,w,h));
        float4 uv = float4(characters[id].x/width,characters[id].y/height,
                           characters[id].width/width, characters[id].height/height);

        RKInPerInstanceAttributesText atomText = RKInPerInstanceAttributesText(position, scale, vertex, uv);
        subdata.push_back(atomText);
      }
    }

    float2 shift(0.0,0.0);
    switch(alignment)
    {
    case RKTextAlignment::center:
    case RKTextAlignment::multiple_values:
      shift = float2(0.0,0.0);
      break;
    case RKTextAlignment::left:
      shift = float2(-rect.center().x(),0.0);
      break;
    case RKTextAlignment::right:
      shift = float2(rect.center().x(),0.0);
      break;
    case RKTextAlignment::top:
      shift = float2(0.0,rect.center().y());
      break;
    case RKTextAlignment::bottom:
      shift = float2(0.0,-rect.center().y());
      break;
    case RKTextAlignment::topLeft:
      shift = float2(-rect.center().x(),rect.center().y());
      break;
    case RKTextAlignment::topRight:
      shift = float2(rect.center().x(),rect.center().y());
      break;
    case RKTextAlignment::bottomLeft:
      shift = float2(-rect.center().x(), -rect.center().y());
      break;
    case RKTextAlignment::bottomRight:
      shift = float2(rect.center().x(), -rect.center().y());
      break;
    }

    for(RKInPerInstanceAttributesText &subdataText: subdata)
    {
      subdataText.vertexCoordinatesData.x -= rect.center().x();
      subdataText.vertexCoordinatesData.y -= rect.center().y();
      subdataText.vertexCoordinatesData.x += shift.x;
      subdataText.vertexCoordinatesData.y += shift.y;

      subdataText.vertexCoordinatesData.x /= 50.0;
      subdataText.vertexCoordinatesData.y /= 50.0;
      subdataText.vertexCoordinatesData.z /= 50.0;
      subdataText.vertexCoordinatesData.w /= 50.0;
    }

    //std::copy(subdata.begin(), subdata.end(), std::inserter(atomData, atomData.end()));
    return subdata;
}
