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

#include "openglenergyvolumerenderedsurface.h"
#include "glgeterror.h"
#include "cubegeometry.h"
#include "rkrenderuniforms.h"

OpenGLEnergyVolumeRenderedSurface::OpenGLEnergyVolumeRenderedSurface(): _isOpenCLInitialized(false)
{

}

void OpenGLEnergyVolumeRenderedSurface::setLogReportingWidget(LogReporting *logReporting)
{
  _logReporter = logReporting;
}

void OpenGLEnergyVolumeRenderedSurface::initializeOpenCL(bool isOpenCLInitialized, cl_context context, cl_device_id deviceId, cl_command_queue commandQueue, QStringList &logData)
{
  _isOpenCLInitialized = isOpenCLInitialized;

  _clContext = context;
  _clDeviceId = deviceId;
  _clCommandQueue = commandQueue;

  _energyGridUnitCell.initialize(isOpenCLInitialized, context, deviceId, commandQueue, logData);
  check_gl_error();
}

void OpenGLEnergyVolumeRenderedSurface::invalidateIsosurface(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  for(const std::shared_ptr<RKRenderStructure> &structure : structures)
  {
     _cache.remove(structure.get());
  }
}

void OpenGLEnergyVolumeRenderedSurface::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteBuffers(size, _vertexBuffer[i].data());
    glDeleteBuffers(size, _indexBuffer[i].data());

    glDeleteVertexArrays(size, _vertexArrayObject[i].data());

    glDeleteTextures(size, _volumeTextures[i].data());
  }
}

void OpenGLEnergyVolumeRenderedSurface::generateBuffers()
{
  _numberOfIndices.resize(_renderStructures.size());

  _volumeTextures.resize(_renderStructures.size());

  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());

  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _numberOfIndices[i].resize(_renderStructures[i].size());

    _volumeTextures[i].resize(_renderStructures[i].size());

    _vertexBuffer[i].resize(_renderStructures[i].size());
    _indexBuffer[i].resize(_renderStructures[i].size());

    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenBuffers(size, _vertexBuffer[i].data());
    glGenBuffers(size, _indexBuffer[i].data());

    glGenTextures(size, _volumeTextures[i].data());

    glGenVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLEnergyVolumeRenderedSurface::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}



void OpenGLEnergyVolumeRenderedSurface::paintGL(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer, GLuint depthTexture)
{
  if(!_isOpenCLInitialized)
      return;

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDepthMask(GL_TRUE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderStructure* renderStructure = dynamic_cast<RKRenderStructure*>(_renderStructures[i][j].get()))
      if (RKRenderAdsorptionSurfaceSource* source = dynamic_cast<RKRenderAdsorptionSurfaceSource*>(_renderStructures[i][j].get()))
      {
        if (_renderStructures[i][j]->isVisible() && source->drawAdsorptionSurface() && source->adsorptionSurfaceRenderingMethod() == RKEnergySurfaceType::volumeRendering)
        {
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_3D, _volumeTextures[i][j]);
          check_gl_error();
          glUniform1i(_volumeTextureUniformLocation, 0);
          check_gl_error();

          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_2D, depthTexture);
          check_gl_error();
          glUniform1i(_depthTextureUniformLocation, 1);
          check_gl_error();


          glActiveTexture(GL_TEXTURE2);
          glBindTexture(GL_TEXTURE_1D, _transferFunctionTexture);
          check_gl_error();
          glUniform1i(_transferFunctionUniformLocation, 2);
          check_gl_error();



          GLfloat m_stepLength=0.001;

          glUniform1f(_stepLengthUniformLocation, m_stepLength);
          check_gl_error();

          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, static_cast<GLintptr>(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));
          glBindBufferRange(GL_UNIFORM_BUFFER, 2, isosurfaceUniformBuffer, static_cast<GLintptr>(index * sizeof(RKIsosurfaceUniforms)), sizeof(RKIsosurfaceUniforms));
          check_gl_error();

          glBindVertexArray(_vertexArrayObject[i][j]);
          check_gl_error();
          glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr);
          check_gl_error();
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}


void OpenGLEnergyVolumeRenderedSurface::reloadData()
{
  initializeVertexArrayObject();
  initializeTransferFunctionTexture();
}


void OpenGLEnergyVolumeRenderedSurface::initializeTransferFunctionTexture()
{
  glDeleteTextures(1, &_transferFunctionTexture);
  check_gl_error();
  glGenTextures(1, &_transferFunctionTexture);
  check_gl_error();
  glBindTexture(GL_TEXTURE_1D, _transferFunctionTexture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16F, transferFunction.size(), 0, GL_RGBA, GL_FLOAT, transferFunction.data());
  check_gl_error();
  glBindTexture(GL_TEXTURE_1D, 0);
}

void OpenGLEnergyVolumeRenderedSurface::initializeVertexArrayObject()
{
  if(!_isOpenCLInitialized)
  {
    return;
  }

  CubeGeometry sphere = CubeGeometry();

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderStructure* renderStructure = dynamic_cast<RKRenderStructure*>(_renderStructures[i][j].get()))
      {
        if (RKRenderAdsorptionSurfaceSource* source = dynamic_cast<RKRenderAdsorptionSurfaceSource*>(_renderStructures[i][j].get()))
        {
          if (source->drawAdsorptionSurface() && source->adsorptionSurfaceRenderingMethod() == RKEnergySurfaceType::volumeRendering )
          {
            std::vector<float4> *textureData{}; // = std::make_shared<std::vector<cl_float>>();

            if(_cache.contains(_renderStructures[i][j].get()))
            {
               std::clock_t beginTime = clock();
               textureData = _cache.object(_renderStructures[i][j].get());
               std::clock_t endTime = clock();
               double elapsedTime = double(endTime - beginTime) * 1000.0 / CLOCKS_PER_SEC;
               _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "Elapsed time for grid-cache lookup " + _renderStructures[i][j]->displayName() + ": " + QString::number(elapsedTime) + " milliseconds.");
            }
            else
            {
              int sizeX = 128;
              int sizeY = 128;
              int sizeZ = 128;

              std::clock_t beginTime = clock();
              double2 probeParameter = source->adsorptionSurfaceProbeParameters();
              std::vector<double3> positions = source->atomUnitCellPositions();
              std::vector<double2> potentialParameters = source->potentialParameters();
              double3x3 unitCell = renderStructure->cell()->unitCell();
              int3 numberOfReplicas = renderStructure->cell()->numberOfReplicas(12.0);
              std::vector<cl_float> gridData = *_energyGridUnitCell.ComputeEnergyGrid(sizeX, sizeY, sizeZ, probeParameter, positions, potentialParameters, unitCell, numberOfReplicas);



              textureData = new std::vector<float4>(128*128*128);

              float minimum=1000.0;
              for(int z=0;z<128;z++)
              {
                for(int y=0;y<128;y++)
                {
                  for(int x=0;x<128;x++)
                  {
                    float value = gridData[x+128*y+z*128*128];
                    if(value<minimum)
                    {
                        minimum=value;
                    }
                  }
                }
              }
              for(int z=0;z<128;z++)
              {
                for(int y=0;y<128;y++)
                {
                  for(int x=0;x<128;x++)
                  {
                    float temp = 1000.0*(1.0/300.0)*(gridData[x+128*y+z*128*128]-minimum);
                    float value;
                    if(temp<0)
                    {
                      value = 0.0;
                    }
                    else if(temp>54000)
                    {
                     value = 1.0;
                    }
                    else
                    {
                      value=temp/65535.0;
                    }


                    // x
                    int xi = (int)(x + 0.5f);
                    float xf = x + 0.5f - xi;
                    float xd0 = Images(gridData,x-1,y,z);
                    float xd1 = Images(gridData,x,y,z);
                    float xd2 = Images(gridData,x+1,y,z);
                    float gx = (xd1 - xd0) * (1.0f - xf) + (xd2 - xd1) * xf; // lerp

                    // y
                    int yi = (int)(y + 0.5f);
                    float yf = y + 0.5f - yi;
                    float yd0 = Images(gridData,x,y-1,z);
                    float yd1 = Images(gridData,x,y,z);
                    float yd2 = Images(gridData,x,y+1,z);
                    float gy = (yd1 - yd0) * (1.0f - yf) + (yd2 - yd1) * yf; // lerp

                    // z
                    int zi = (int)(z + 0.5f);
                    float zf = z + 0.5f - zi;
                    float zd0 =  Images(gridData,x,y,z-1);
                    float zd1 =  Images(gridData,x,y,z);
                    float zd2 =  Images(gridData,x,y,z+1);
                    float gz =  (zd1 - zd0) * (1.0f - zf) + (zd2 - zd1) * zf; // lerp

/*
                    // Sobel 3D gradient (finite difference leads to anisotropic gradients)
                    float gx = Images(gridData,x+1,y+1,z+1)+2*Images(gridData,x,y+1,z+1)+Images(gridData,x-1,y+1,z+1) +2*Images(gridData,x+1,y+1,z)+4*Images(gridData,x,y+1,z)+ 2*Images(gridData,x-1,y+1,z) +
                               Images(gridData,x+1,y+1,z-1)+2*Images(gridData,x,y+1,z-1)+Images(gridData,x-1,y+1,z-1) - (Images(gridData,x+1,y-1,z+1)+2*Images(gridData,x,y-1,z+1)+Images(gridData,x-1,y-1,z+1) +
                               2*Images(gridData,x+1,y-1,z)+4*Images(gridData,x,y-1,z)+ 2*Images(gridData,x-1,y-1,z) +Images(gridData,x+1,y-1,z-1)+2*Images(gridData,x,y-1,z-1)+Images(gridData,x-1,y-1,z-1)) ;

                    float gy = Images(gridData,x+1,y+1,z+1)+2*Images(gridData,x+1,y+1,z)+Images(gridData,x+1,y+1,z-1) +2*Images(gridData,x+1,y,z+1)+4*Images(gridData,x+1,y,z)+ 2*Images(gridData,x+1,y,z-1) +
                               Images(gridData,x+1,y-1,z+1)+2*Images(gridData,x+1,y-1,z)+Images(gridData,x+1,y-1,z-1) -(Images(gridData,x-1,y+1,z+1)+2*Images(gridData,x-1,y+1,z)+Images(gridData,x-1,y+1,z-1) +
                               2*Images(gridData,x-1,y,z+1)+4*Images(gridData,x-1,y,z)+ 2*Images(gridData,x-1,y,z-1) + Images(gridData,x-1,y-1,z+1)+2*Images(gridData,x-1,y-1,z)+Images(gridData,x-1,y-1,z-1));

                    float gz = Images(gridData,x+1,y+1,z+1)+2*Images(gridData,x,y+1,z+1)+Images(gridData,x-1,y+1,z+1) +2*Images(gridData,x+1,y,z+1)+4*Images(gridData,x,y,z+1)+2*Images(gridData,x-1,y,z+1) +
                               Images(gridData,x+1,y-1,z+1)+2*Images(gridData,x,y-1,z+1)+Images(gridData,x-1,y-1,z+1) -(Images(gridData,x+1,y+1,z-1)+2*Images(gridData,x,y+1,z-1)+Images(gridData,x-1,y+1,z-1) +
                               2*Images(gridData,x+1,y,z-1)+4*Images(gridData,x,y,z-1)+ 2*Images(gridData,x-1,y,z-1) +Images(gridData,x+1,y-1,z-1)+2*Images(gridData,x,y-1,z-1)+Images(gridData,x-1,y-1,z-1));
*/
                    (*textureData)[x+128*y+z*128*128] = float4(value, gx, gy, gz);
                  }
                }
              }

              _cache.insert(_renderStructures[i][j].get(),textureData);
            }

            glBindTexture(GL_TEXTURE_3D, _volumeTextures[i][j]);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            check_gl_error();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // The array on the host has 1 byte alignment
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 128, 128, 128, 0, GL_RGBA, GL_FLOAT, textureData->data());
            check_gl_error();
            glBindTexture(GL_TEXTURE_3D, 0);
            check_gl_error();

            glBindVertexArray(_vertexArrayObject[i][j]);
            check_gl_error();

            _numberOfIndices[i][j] = sphere.indices().size();

            glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);
            check_gl_error();

            if(sphere.vertices().size()>0)
            {
              glBufferData(GL_ARRAY_BUFFER, sphere.vertices().size() * sizeof(RKVertex), sphere.vertices().data(), GL_DYNAMIC_DRAW);
            }
            check_gl_error();

            glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
            check_gl_error();

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer[i][j]);
            check_gl_error();
            if(sphere.indices().size()>0)
            {
              glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices().size() * sizeof(GLshort), sphere.indices().data(), GL_DYNAMIC_DRAW);
            }
            check_gl_error();

            glEnableVertexAttribArray(_vertexPositionAttributeLocation);
            check_gl_error();

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
          }
        }
      }
    }
  }
}

void OpenGLEnergyVolumeRenderedSurface::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLEnergyVolumeRenderedSurface::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLEnergyVolumeRenderedSurface::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _program = glCreateProgram();
    check_gl_error();

    glAttachShader(_program, vertexShader);
    check_gl_error();
    glAttachShader(_program, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_program, 0, "vFragColor");

    linkProgram(_program);

    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");

    _transferFunctionUniformLocation   = glGetUniformLocation(_program, "transferFunction");
    _volumeTextureUniformLocation   = glGetUniformLocation(_program, "volume");
    _depthTextureUniformLocation   = glGetUniformLocation(_program, "depthTexture");

    _stepLengthUniformLocation   = glGetUniformLocation(_program, "step_length");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_transferFunctionUniformLocation < 0) qDebug() << "Shader did not contain the 'transferFunction' uniform.";
    if (_volumeTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'volume' uniform.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

void OpenGLEnergyVolumeRenderedSurface::initializeTransformUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
}

void OpenGLEnergyVolumeRenderedSurface::initializeStructureUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
}

void OpenGLEnergyVolumeRenderedSurface::initializeIsosurfaceUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "IsosurfaceUniformBlock"), 2);
  check_gl_error();
}

void OpenGLEnergyVolumeRenderedSurface::initializeLightUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
}


// Based on: page 168 "Real-Time Volume Graphics", Klaus Engels et al.

const std::string  OpenGLEnergyVolumeRenderedSurface::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLIsosurfaceUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;

// Inputs from vertex shader
out VS_OUT
{
  smooth vec3 UV;
  smooth vec3 position;
  smooth vec3 L;
  smooth vec4 clipPosition;
} vs_out;

void main()
{
  vec4 pos = structureUniforms.modelMatrix * structureUniforms.boxMatrix * vertexPosition;
  vs_out.position = pos.xyz;
  vs_out.UV =  vertexPosition.xyz;
  vs_out.clipPosition = frameUniforms.mvpMatrix * pos;

  vec4 P = frameUniforms.viewMatrix *  pos;

  // Calculate light vector
  vec4 dir = lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w;

  gl_Position = frameUniforms.mvpMatrix * pos;
}
)foo";

const std::string  OpenGLEnergyVolumeRenderedSurface::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLIsosurfaceUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
out vec4 vFragColor;
// Input from vertex shader

in VS_OUT
{
  smooth vec3 UV;
  smooth vec3 position;
  smooth vec3 L;
  smooth vec4 clipPosition;
} fs_in;

uniform sampler3D volume;
uniform sampler1D transferFunction;
uniform sampler2D depthTexture;

// Ray
struct Ray
{
  vec3 origin;
  vec3 direction;
};

// Axis-aligned bounding box
struct AABB
{
  vec3 top;
  vec3 bottom;
};

// Slab method for ray-box intersection
void ray_box_intersection(Ray ray, AABB box, out float t_0, out float t_1)
{
  vec3 direction_inv = 1.0 / ray.direction;
  vec3 t_top = direction_inv * (box.top - ray.origin);
  vec3 t_bottom = direction_inv * (box.bottom - ray.origin);
  vec3 t_min = min(t_top, t_bottom);
  vec2 t = max(t_min.xx, t_min.yz);
  t_0 = max(0.0, max(t.x, t.y));
  vec3 t_max = max(t_top, t_bottom);
  t = min(t_max.xx, t_max.yz);
  t_1 = min(t.x, t.y);
}


// A very simple colour transfer function
vec4 colour_transfer(float intensity)
{
  vec3 high = vec3(1.0, 1.0, 1.0);
  vec3 low = vec3(0.0, 0.0, 0.0);
  float alpha = (exp(intensity) - 1.0) / (exp(1.0) - 1.0);
  return vec4(intensity * high + (1.0 - intensity) * low, alpha);
}

vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


const int numSamples = 100000;

void main()
{
  vec3 ambient, diffuse, specular;
  vec3 numberOfReplicas = structureUniforms.numberOfReplicas.xyz;
  vec3 direction = normalize(fs_in.position.xyz - frameUniforms.cameraPosition.xyz);
  vec4 dir = vec4(direction.x,direction.y,direction.z,0.0);
  vec3 ray_direction = (structureUniforms.inverseBoxMatrix * structureUniforms.inverseModelMatrix * dir).xyz;

  vec3 ray_origin = fs_in.UV;

  float stepLength = isosurfaceUniforms.stepLength/numberOfReplicas.z;

  float t_0, t_1;
  Ray casting_ray = Ray(ray_origin, ray_direction);
  AABB bounding_box = AABB(vec3(1.0,1.0,1.0), vec3(0.0,0.0,0.0));
  ray_box_intersection(casting_ray, bounding_box, t_0, t_1);

  vec3 ray_start = ray_origin + ray_direction * t_0;
  vec3 ray_stop = ray_origin + ray_direction * t_1;

  vec3 ray = ray_stop - ray_start;
  float ray_length = length(ray);
  vec3 step_vector = stepLength * ray / ray_length;


  float depth = texelFetch(depthTexture, ivec2(gl_FragCoord.xy),0).r;
  gl_FragDepth = depth;  // at least write a value once (required)
  float newDepth = 1.0;
  mat4 m = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * isosurfaceUniforms.boxMatrix;

  vec4 colour = vec4(0.0,0.0,0.0,0.0);
  vec3 position = ray_start;
  for (int i=0; i < numSamples && ray_length > 0 && colour.a < 1.0; i++)
  {
    vec4 values = texture(volume,numberOfReplicas * position);
    vec3 normal = normalize((structureUniforms.modelMatrix * transpose(structureUniforms.inverseBoxMatrix) * vec4(values.gba,0.0)).rgb);

    //vec4 c = colour_transfer(values.r);
    vec4 c = texture(transferFunction,values.r);


    vec3 R = reflect(-direction, normal);
    ambient = vec3(0.1,0.1,0.1);
    float dotProduct = dot(normal, direction);

    if(dotProduct < 0)
    {
      ambient = isosurfaceUniforms.ambientBackSide.rgb;
      diffuse = vec3(max(abs(dotProduct),0.0)) * isosurfaceUniforms.diffuseBackSide.rgb;
      specular = vec3(pow(max(dot(R, direction), 0.0), isosurfaceUniforms.shininessBackSide)) * isosurfaceUniforms.specularBackSide.rgb;
      vec3 totalColor = (ambient+diffuse+specular).rgb;

      if (isosurfaceUniforms.backHDR)
      {
        totalColor = 1.0 - exp2(-totalColor * isosurfaceUniforms.backHDRExposure);
      }

      // Alpha-blending
      colour.rgb = c.a * totalColor * c.rgb + (1 - c.a) * colour.a * colour.rgb;
      colour.a = c.a + (1 - c.a) * colour.a;
    }
    else
    {
      ambient = isosurfaceUniforms.ambientFrontSide.rgb;
      diffuse = vec3(max(abs(dotProduct),0.0)) * isosurfaceUniforms.diffuseFrontSide.rgb;
      specular = vec3(pow(max(dot(R, direction), 0.0), isosurfaceUniforms.shininessFrontSide)) * isosurfaceUniforms.specularFrontSide.rgb;
      vec3 totalColor = (ambient+diffuse+specular).rgb;

      if (isosurfaceUniforms.frontHDR)
      {
        totalColor = 1.0 - exp2(-totalColor * isosurfaceUniforms.frontHDRExposure);
      }

      // Alpha-blending
      colour.rgb = c.a * totalColor * c.rgb + (1 - c.a) * colour.a * colour.rgb;
      colour.a = c.a + (1 - c.a) * colour.a;
    }

    position = position + step_vector;
    ray_length -= stepLength;

    vec4 clipPosition = m * vec4(position,1.0);
    newDepth = 0.5*(clipPosition.z / clipPosition.w)+0.5;
    if(newDepth>depth)
    {
      break;
    }
  }

  if(colour.a<0.99)
  {
    discard;
  }

  gl_FragDepth = newDepth;

  vec3 hsv = rgb2hsv(colour.xyz);
  hsv.x = hsv.x * isosurfaceUniforms.hue;
  hsv.y = hsv.y * isosurfaceUniforms.saturation;
  hsv.z = hsv.z * isosurfaceUniforms.value;
  vFragColor = vec4(hsv2rgb(hsv),colour.a);
}
)foo";

std::array<float4, 256> OpenGLEnergyVolumeRenderedSurface::transferFunction
{
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0),
    float4(0.8, 0.7, 0.2, 0.0252),
    float4(0.8, 0.7, 0.2, 0.1148),
    float4(0.8, 0.7, 0.2, 0.2044),
    float4(0.8, 0.7, 0.2, 0.294),
    float4(0.8, 0.7, 0.2, 0.3836),
    float4(0.8, 0.7, 0.2, 0.4732),
    float4(0.8, 0.7, 0.2, 0.5628),
    float4(0.8, 0.7, 0.2, 0.6524),
    float4(0.8, 0.7, 0.2, 0.701029),
    float4(0.8, 0.7, 0.2, 0.703223),
    float4(0.8, 0.7, 0.2, 0.705417),
    float4(0.8, 0.7, 0.2, 0.707611),
    float4(0.8, 0.7, 0.2, 0.709806),
    float4(0.8, 0.7, 0.2, 0.712),
    float4(0.8, 0.7, 0.2, 0.714194),
    float4(0.8, 0.7, 0.2, 0.716389),
    float4(0.8, 0.7, 0.2, 0.718583),
    float4(0.8, 0.7, 0.2, 0.720777),
    float4(0.8, 0.7, 0.2, 0.722971),
    float4(0.8, 0.7, 0.2, 0.725166),
    float4(0.8, 0.7, 0.2, 0.72736),
    float4(0.8, 0.7, 0.2, 0.729554),
    float4(0.8, 0.7, 0.2, 0.731749),
    float4(0.8, 0.7, 0.2, 0.733943),
    float4(0.8, 0.7, 0.2, 0.736137),
    float4(0.8, 0.7, 0.2, 0.738331),
    float4(0.8, 0.7, 0.2, 0.740526),
    float4(0.8, 0.7, 0.2, 0.74272),
    float4(0.8, 0.7, 0.2, 0.744914),
    float4(0.8, 0.7, 0.2, 0.747109),
    float4(0.8, 0.7, 0.2, 0.749303),
    float4(0.8, 0.7, 0.2, 0.751497),
    float4(0.8, 0.7, 0.2, 0.753691),
    float4(0.8, 0.7, 0.2, 0.755886),
    float4(0.8, 0.7, 0.2, 0.75808),
    float4(0.8, 0.7, 0.2, 0.760274),
    float4(0.8, 0.7, 0.2, 0.762469),
    float4(0.8, 0.7, 0.2, 0.764663),
    float4(0.8, 0.7, 0.2, 0.766857),
    float4(0.8, 0.7, 0.2, 0.769051),
    float4(0.8, 0.7, 0.2, 0.771246),
    float4(0.8, 0.7, 0.2, 0.77344),
    float4(0.8, 0.7, 0.2, 0.775634),
    float4(0.8, 0.7, 0.2, 0.777829),
    float4(0.8, 0.7, 0.2, 0.780023),
    float4(0.8, 0.7, 0.2, 0.782217),
    float4(0.8, 0.7, 0.2, 0.784411),
    float4(0.8, 0.7, 0.2, 0.786606),
    float4(0.8, 0.7, 0.2, 0.7888),
    float4(0.8, 0.7, 0.2, 0.790994),
    float4(0.8, 0.7, 0.2, 0.793189),
    float4(0.8, 0.7, 0.2, 0.795383),
    float4(0.8, 0.7, 0.2, 0.797577),
    float4(0.8, 0.7, 0.2, 0.799771),
    float4(0.8, 0.7, 0.2, 0.801966),
    float4(0.8, 0.7, 0.2, 0.80416),
    float4(0.8, 0.7, 0.2, 0.806354),
    float4(0.8, 0.7, 0.2, 0.808549),
    float4(0.8, 0.7, 0.2, 0.810743),
    float4(0.8, 0.7, 0.2, 0.812937),
    float4(0.8, 0.7, 0.2, 0.815131),
    float4(0.8, 0.7, 0.2, 0.817326),
    float4(0.8, 0.7, 0.2, 0.81952),
    float4(0.8, 0.7, 0.2, 0.821714),
    float4(0.8, 0.7, 0.2, 0.823909),
    float4(0.8, 0.7, 0.2, 0.826103),
    float4(0.8, 0.7, 0.2, 0.828297),
    float4(0.8, 0.7, 0.2, 0.830491),
    float4(0.8, 0.7, 0.2, 0.832686),
    float4(0.8, 0.7, 0.2, 0.83488),
    float4(0.8, 0.7, 0.2, 0.837074),
    float4(0.8, 0.7, 0.2, 0.839269),
    float4(0.8, 0.7, 0.2, 0.841463),
    float4(0.8, 0.7, 0.2, 0.843657),
    float4(0.799556, 0.699822, 0.200711, 0.845851),
    float4(0.792444, 0.696978, 0.212089, 0.848046),
    float4(0.785333, 0.694133, 0.223467, 0.85024),
    float4(0.778222, 0.691289, 0.234844, 0.852434),
    float4(0.771111, 0.688444, 0.246222, 0.854629),
    float4(0.764, 0.6856, 0.2576, 0.856823),
    float4(0.756889, 0.682756, 0.268978, 0.859017),
    float4(0.749778, 0.679911, 0.280356, 0.861211),
    float4(0.742667, 0.677067, 0.291733, 0.863406),
    float4(0.735556, 0.674222, 0.303111, 0.8656),
    float4(0.728444, 0.671378, 0.314489, 0.867794),
    float4(0.721333, 0.668533, 0.325867, 0.869989),
    float4(0.714222, 0.665689, 0.337244, 0.872183),
    float4(0.707111, 0.662844, 0.348622, 0.874377),
    float4(0.7, 0.66, 0.36, 0.876571),
    float4(0.692889, 0.657156, 0.371378, 0.878766),
    float4(0.685778, 0.654311, 0.382756, 0.88096),
    float4(0.678667, 0.651467, 0.394133, 0.883154),
    float4(0.671556, 0.648622, 0.405511, 0.885349),
    float4(0.664444, 0.645778, 0.416889, 0.887543),
    float4(0.657333, 0.642933, 0.428267, 0.889737),
    float4(0.650222, 0.640089, 0.439644, 0.891931),
    float4(0.643111, 0.637244, 0.451022, 0.894126),
    float4(0.636, 0.6344, 0.4624, 0.89632),
    float4(0.628889, 0.631556, 0.473778, 0.898514),
    float4(0.621778, 0.628711, 0.485156, 0.900709),
    float4(0.614667, 0.625867, 0.496533, 0.902903),
    float4(0.607556, 0.623022, 0.507911, 0.905097),
    float4(0.600444, 0.620178, 0.519289, 0.907291),
    float4(0.593333, 0.617333, 0.530667, 0.909486),
    float4(0.586222, 0.614489, 0.542044, 0.91168),
    float4(0.579111, 0.611644, 0.553422, 0.913874),
    float4(0.572, 0.6088, 0.5648, 0.916069),
    float4(0.564889, 0.605956, 0.576178, 0.918263),
    float4(0.557778, 0.603111, 0.587556, 0.920457),
    float4(0.550667, 0.600267, 0.598933, 0.922651),
    float4(0.543556, 0.597422, 0.610311, 0.924846),
    float4(0.536444, 0.594578, 0.621689, 0.92704),
    float4(0.529333, 0.591733, 0.633067, 0.929234),
    float4(0.522222, 0.588889, 0.644444, 0.931429),
    float4(0.515111, 0.586044, 0.655822, 0.933623),
    float4(0.508, 0.5832, 0.6672, 0.935817),
    float4(0.500889, 0.580356, 0.678578, 0.938011),
    float4(0.493778, 0.577511, 0.689956, 0.940206),
    float4(0.486667, 0.574667, 0.701333, 0.9424),
    float4(0.479556, 0.571822, 0.712711, 0.944594),
    float4(0.472444, 0.568978, 0.724089, 0.946789),
    float4(0.465333, 0.566133, 0.735467, 0.948983),
    float4(0.458222, 0.563289, 0.746844, 0.951177),
    float4(0.451111, 0.560444, 0.758222, 0.953371),
    float4(0.444, 0.5576, 0.7696, 0.955566),
    float4(0.436889, 0.554756, 0.780978, 0.95776),
    float4(0.429778, 0.551911, 0.792356, 0.959954),
    float4(0.422667, 0.549067, 0.803733, 0.962149),
    float4(0.415556, 0.546222, 0.815111, 0.964343),
    float4(0.408444, 0.543378, 0.826489, 0.966537),
    float4(0.401333, 0.540533, 0.837867, 0.968731),
    float4(0.394222, 0.537689, 0.849244, 0.970926),
    float4(0.387111, 0.534844, 0.860622, 0.97312),
    float4(0.38, 0.532, 0.872, 0.975314),
    float4(0.372889, 0.529156, 0.883378, 0.977509),
    float4(0.365778, 0.526311, 0.894756, 0.979703),
    float4(0.358667, 0.523467, 0.906133, 0.981897),
    float4(0.351556, 0.520622, 0.917511, 0.984091),
    float4(0.344444, 0.517778, 0.928889, 0.986286),
    float4(0.337333, 0.514933, 0.940267, 0.98848),
    float4(0.330222, 0.512089, 0.951644, 0.990674),
    float4(0.323111, 0.509244, 0.963022, 0.992869),
    float4(0.316, 0.5064, 0.9744, 0.995063),
    float4(0.308889, 0.503556, 0.985778, 0.997257),
    float4(0.301778, 0.500711, 0.997156, 0.999451),
    float4(0.3, 0.5, 1, 0.99232),
    float4(0.3, 0.5, 1, 0.98208),
    float4(0.3, 0.5, 1, 0.97184),
    float4(0.3, 0.5, 1, 0.9616),
    float4(0.3, 0.5, 1, 0.95136),
    float4(0.3, 0.5, 1, 0.94112),
    float4(0.3, 0.5, 1, 0.93088),
    float4(0.3, 0.5, 1, 0.92064),
    float4(0.3, 0.5, 1, 0.9104),
    float4(0.3, 0.5, 1, 0.90016),
    float4(0.3, 0.5, 1, 0.88992),
    float4(0.3, 0.5, 1, 0.87968),
    float4(0.3, 0.5, 1, 0.86944),
    float4(0.3, 0.5, 1, 0.8592),
    float4(0.3, 0.5, 1, 0.84896),
    float4(0.3, 0.5, 1, 0.83872),
    float4(0.3, 0.5, 1, 0.82848),
    float4(0.3, 0.5, 1, 0.81824),
    float4(0.3, 0.5, 1, 0.808),
    float4(0.3, 0.5, 1, 0.79776),
    float4(0.3, 0.5, 1, 0.78752),
    float4(0.3, 0.5, 1, 0.77728),
    float4(0.3, 0.5, 1, 0.76704),
    float4(0.3, 0.5, 1, 0.7568),
    float4(0.3, 0.5, 1, 0.74656),
    float4(0.3, 0.5, 1, 0.73632),
    float4(0.3, 0.5, 1, 0.72608),
    float4(0.3, 0.5, 1, 0.71584),
    float4(0.3, 0.5, 1, 0.7056),
    float4(0.3, 0.5, 1, 0.69536),
    float4(0.3, 0.5, 1, 0.68512),
    float4(0.3, 0.5, 1, 0.67488),
    float4(0.3, 0.5, 1, 0.66464),
    float4(0.3, 0.5, 1, 0.6544),
    float4(0.3, 0.5, 1, 0.64416),
    float4(0.3, 0.5, 1, 0.63392),
    float4(0.3, 0.5, 1, 0.62368),
    float4(0.3, 0.5, 1, 0.61344),
    float4(0.3, 0.5, 1, 0.6032),
    float4(0.3, 0.5, 1, 0.59296),
    float4(0.3, 0.5, 1, 0.58272),
    float4(0.3, 0.5, 1, 0.57248),
    float4(0.3, 0.5, 1, 0.56224),
    float4(0.3, 0.5, 1, 0.552),
    float4(0.3, 0.5, 1, 0.54176),
    float4(0.3, 0.5, 1, 0.53152),
    float4(0.3, 0.5, 1, 0.52128),
    float4(0.3, 0.5, 1, 0.51104),
    float4(0.3, 0.5, 1, 0.5008),
    float4(0.3, 0.5, 1, 0.49056),
    float4(0.3, 0.5, 1, 0.48032),
    float4(0.3, 0.5, 1, 0.47008),
    float4(0.3, 0.5, 1, 0.45984),
    float4(0.3, 0.5, 1, 0.4496),
    float4(0.3, 0.5, 1, 0.43936),
    float4(0.3, 0.5, 1, 0.42912),
    float4(0.3, 0.5, 1, 0.41888),
    float4(0.3, 0.5, 1, 0.40864),
    float4(0.3, 0.5, 1, 0.3984),
    float4(0.3, 0.5, 1, 0.38816),
    float4(0.3, 0.5, 1, 0.37792),
    float4(0.3, 0.5, 1, 0.36768),
    float4(0.3, 0.5, 1, 0.35744),
    float4(0.3, 0.5, 1, 0.3472),
    float4(0.3, 0.5, 1, 0.33696),
    float4(0.3, 0.5, 1, 0.32672),
    float4(0.3, 0.5, 1, 0.31648),
    float4(0.3, 0.5, 1, 0.30624),
    float4(0.3, 0.5, 1, 0.296),
    float4(0.3, 0.5, 1, 0.28576),
    float4(0.3, 0.5, 1, 0.27552),
    float4(0.3, 0.5, 1, 0.26528),
    float4(0.3, 0.5, 1, 0.25504),
    float4(0.3, 0.5, 1, 0.2448),
    float4(0.3, 0.5, 1, 0.23456),
    float4(0.3, 0.5, 1, 0.22432),
    float4(0.3, 0.5, 1, 0.21408),
    float4(0.3, 0.5, 1, 0.20384),
    float4(0.3, 0.5, 1, 0.1936),
    float4(0.3, 0.5, 1, 0.18336),
    float4(0.3, 0.5, 1, 0.17312),
    float4(0.3, 0.5, 1, 0.16288),
    float4(0.3, 0.5, 1, 0.15264),
    float4(0.3, 0.5, 1, 0.1424),
    float4(0.3, 0.5, 1, 0.13216),
    float4(0.3, 0.5, 1, 0.12192),
    float4(0.3, 0.5, 1, 0.11168),
    float4(0.3, 0.5, 1, 0.10144),
    float4(0.3, 0.5, 1, 0.0912),
    float4(0.3, 0.5, 1, 0.08096),
    float4(0.3, 0.5, 1, 0.07072),
    float4(0.3, 0.5, 1, 0.06048),
    float4(0.3, 0.5, 1, 0.05024),
    float4(0.3, 0.5, 1, 0.04),
    float4(0.3, 0.5, 1, 0.02976),
    float4(0.3, 0.5, 1, 0.01952),
    float4(0.3, 0.5, 1, 0.00928),
    float4(0.3, 0.5, 1, 0),
    float4(0.3, 0.5, 1, 0)
};

