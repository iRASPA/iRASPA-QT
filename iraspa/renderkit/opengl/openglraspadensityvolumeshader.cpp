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

#include "glgeterror.h"
#include "openglraspadensityvolumeshader.h"
#include "unitcubegeometry.h"
#include "spheregeometry.h"
#include "cubegeometry.h"
#include <qmath.h>
#include <math.h>
#include <cmath>


OpenGLRASPADensityVolumeShader::OpenGLRASPADensityVolumeShader()
{

}


void OpenGLRASPADensityVolumeShader::deleteBuffers()
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

void OpenGLRASPADensityVolumeShader::generateBuffers()
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

void OpenGLRASPADensityVolumeShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}



void OpenGLRASPADensityVolumeShader::paintGL(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer, GLuint depthTexture)
{
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
      if (std::shared_ptr<RKRenderRASPADensityVolumeSource> object = std::dynamic_pointer_cast<RKRenderRASPADensityVolumeSource>(_renderStructures[i][j]))
      {
        if(_renderStructures[i][j]->isVisible())
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

          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));
          check_gl_error();
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


void OpenGLRASPADensityVolumeShader::reloadData()
{
  initializeVertexArrayObject();
  initializeTransferFunctionTexture();
}

void OpenGLRASPADensityVolumeShader::initializeTransferFunctionTexture()
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

void OpenGLRASPADensityVolumeShader::initializeVertexArrayObject()
{
    CubeGeometry sphere = CubeGeometry();

    for(size_t i=0;i<_renderStructures.size();i++)
    {
      for(size_t j=0;j<_renderStructures[i].size();j++)
      {
        if (std::shared_ptr<RKRenderRASPADensityVolumeSource> object = std::dynamic_pointer_cast<RKRenderRASPADensityVolumeSource>(_renderStructures[i][j]))
        {
          glBindTexture(GL_TEXTURE_3D, _volumeTextures[i][j]);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          check_gl_error();
          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // The array on the host has 1 byte alignment
          glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, object->dimension().x, object->dimension().y, object->dimension().z, 0, GL_RGBA, GL_FLOAT, object->data().data());
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

void OpenGLRASPADensityVolumeShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLRASPADensityVolumeShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLRASPADensityVolumeShader::_fragmentShaderSource.c_str());

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

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_transferFunctionUniformLocation < 0) qDebug() << "Shader did not contain the 'transferFunction' uniform.";
    if (_volumeTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'volume' uniform.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

void OpenGLRASPADensityVolumeShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
}

void OpenGLRASPADensityVolumeShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
}

void OpenGLRASPADensityVolumeShader::initializeLightUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
}


// Based on: page 168 "Real-Time Volume Graphics", Klaus Engels et al.

const std::string  OpenGLRASPADensityVolumeShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
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

const std::string  OpenGLRASPADensityVolumeShader::_fragmentShaderSource =
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

const int numSamples = 100000;

void main()
{
  vec3 numberOfReplicas = structureUniforms.numberOfReplicas.xyz;
  vec3 direction = normalize(fs_in.position.xyz - frameUniforms.cameraPosition.xyz);
  vec4 dir = vec4(direction.x,direction.y,direction.z,0.0);
  vec3 ray_direction = (structureUniforms.inverseBoxMatrix * structureUniforms.inverseModelMatrix * dir).xyz;

  vec3 ray_origin = fs_in.UV;

  float stepLength = 0.001/numberOfReplicas.z;
  //float stepLength = isosurfaceUniforms.stepLength/numberOfReplicas.z;

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
  mat4 m = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * structureUniforms.boxMatrix;

  vec4 colour = vec4(0.0,0.0,0.0,0.0);
  vec3 position = ray_start;
  for (int i=0; i < numSamples && ray_length > 0 && colour.a < 1.0; i++)
  {
    vec4 values = texture(volume,numberOfReplicas * position);
    vec3 normal = normalize((structureUniforms.modelMatrix * transpose(structureUniforms.inverseBoxMatrix) * vec4(values.gba,0.0)).rgb);

    //vec4 c = colour_transfer(values.r);
    vec4 c = texture(transferFunction,values.r);


    vec3 R = reflect(-direction, normal);
    vec3 ambient = vec3(0.3,0.3,0.3);
    vec3 diffuse = vec3(max(abs(dot(normal, direction)),0.0));
    vec3 specular = vec3(pow(max(dot(R, direction), 0.0), 0.4));

    // Alpha-blending
    colour.rgb = c.a * (ambient+diffuse+specular) * c.rgb + (1 - c.a) * colour.a * colour.rgb;
    colour.a = c.a + (1 - c.a) * colour.a;

    position = position + step_vector;
    ray_length -= stepLength;

    vec4 clipPosition = m * vec4(position,1.0);
    newDepth = 0.5*(clipPosition.z / clipPosition.w)+0.5;
    if(newDepth>depth)
    {
      break;
    }
  }

  //if(colour.a<0.99)
  //{
  //  discard;
  //}

  if(colour.a>0.9)
  {
    gl_FragDepth = newDepth;
  }

  vFragColor.rgb = 1.0 - exp2(-colour.rgb * 1.5);
  vFragColor.a=colour.a;
}
)foo";

std::array<float4, 256> OpenGLRASPADensityVolumeShader::transferFunction
{
    float4(0.5, 0.2, 0.2, 0),
    float4(0.55102, 0.210204, 0.210204, 0),
    float4(0.602241, 0.220448, 0.220448, 0),
    float4(0.653461, 0.230692, 0.230692, 0),
    float4(0.704682, 0.240936, 0.240936, 0),
    float4(0.755902, 0.25118, 0.25118, 0),
    float4(0.807123, 0.261425, 0.261425, 0),
    float4(0.858343, 0.271669, 0.271669, 0),
    float4(0.909564, 0.281913, 0.281913, 0),
    float4(0.960784, 0.292157, 0.292157, 0),
    float4(0.984571, 0.312, 0.305143, 0),
    float4(0.918743, 0.3632, 0.327086, 0),
    float4(0.852914, 0.4144, 0.349029, 0),
    float4(0.787086, 0.4656, 0.370971, 0),
    float4(0.721257, 0.5168, 0.392914, 0),
    float4(0.655429, 0.568, 0.414857, 0),
    float4(0.5896, 0.6192, 0.4368, 0),
    float4(0.523771, 0.6704, 0.458743, 0),
    float4(0.457943, 0.7216, 0.480686, 0),
    float4(0.392114, 0.7728, 0.502629, 0),
    float4(0.326286, 0.824, 0.524571, 0),
    float4(0.260457, 0.8752, 0.546514, 0),
    float4(0.194629, 0.9264, 0.568457, 0),
    float4(0.1288, 0.9776, 0.5904, 0),
    float4(0.1, 0.99136, 0.59712, 0),
    float4(0.1, 0.976, 0.592, 0),
    float4(0.1, 0.96064, 0.58688, 0),
    float4(0.1, 0.94528, 0.58176, 0),
    float4(0.1, 0.92992, 0.57664, 0),
    float4(0.1, 0.91456, 0.57152, 0),
    float4(0.1, 0.8992, 0.5664, 0),
    float4(0.1, 0.88384, 0.56128, 0),
    float4(0.1, 0.86848, 0.55616, 0),
    float4(0.1, 0.85312, 0.55104, 0),
    float4(0.1, 0.83776, 0.54592, 0),
    float4(0.1, 0.8224, 0.5408, 0),
    float4(0.1, 0.80704, 0.53568, 0),
    float4(0.1, 0.79168, 0.53056, 0),
    float4(0.1, 0.77632, 0.52544, 0),
    float4(0.1, 0.76096, 0.52032, 0),
    float4(0.1, 0.7456, 0.5152, 0),
    float4(0.1, 0.73024, 0.51008, 0),
    float4(0.1, 0.71488, 0.50496, 0),
    float4(0.1, 0.699893, 0.500267, 0),
    float4(0.1, 0.69648, 0.5088, 0),
    float4(0.1, 0.693067, 0.517333, 0),
    float4(0.1, 0.689653, 0.525867, 0),
    float4(0.1, 0.68624, 0.5344, 0),
    float4(0.1, 0.682827, 0.542933, 0),
    float4(0.1, 0.679413, 0.551467, 0),
    float4(0.1, 0.676, 0.56, 0),
    float4(0.1, 0.672587, 0.568533, 0),
    float4(0.1, 0.669173, 0.577067, 0),
    float4(0.1, 0.66576, 0.5856, 0),
    float4(0.1, 0.662347, 0.594133, 0),
    float4(0.1, 0.658933, 0.602667, 0),
    float4(0.1, 0.65552, 0.6112, 0),
    float4(0.1, 0.652107, 0.619733, 0),
    float4(0.1, 0.648693, 0.628267, 0),
    float4(0.1, 0.64528, 0.6368, 0.00416),
    float4(0.1, 0.641867, 0.645333, 0.0144),
    float4(0.1, 0.638453, 0.653867, 0.02464),
    float4(0.1, 0.63504, 0.6624, 0.03488),
    float4(0.1, 0.631627, 0.670933, 0.04512),
    float4(0.1, 0.628213, 0.679467, 0.05536),
    float4(0.1, 0.6248, 0.688, 0.0656),
    float4(0.1, 0.621387, 0.696533, 0.07584),
    float4(0.1, 0.617973, 0.705067, 0.08608),
    float4(0.1, 0.61456, 0.7136, 0.09632),
    float4(0.1, 0.611147, 0.722133, 0.10656),
    float4(0.1, 0.607733, 0.730667, 0.1168),
    float4(0.1, 0.60432, 0.7392, 0.12704),
    float4(0.1, 0.600907, 0.747733, 0.13728),
    float4(0.1, 0.597493, 0.756267, 0.14752),
    float4(0.1, 0.59408, 0.7648, 0.15776),
    float4(0.1, 0.590667, 0.773333, 0.168),
    float4(0.1, 0.587253, 0.781867, 0.17824),
    float4(0.1, 0.58384, 0.7904, 0.18848),
    float4(0.1, 0.580427, 0.798933, 0.19872),
    float4(0.1, 0.577013, 0.807467, 0.20896),
    float4(0.1, 0.5736, 0.816, 0.2192),
    float4(0.1, 0.570187, 0.824533, 0.22944),
    float4(0.1, 0.566773, 0.833067, 0.23968),
    float4(0.1, 0.56336, 0.8416, 0.24992),
    float4(0.1, 0.559947, 0.850133, 0.26016),
    float4(0.1, 0.556533, 0.858667, 0.2704),
    float4(0.1, 0.55312, 0.8672, 0.28064),
    float4(0.1, 0.549707, 0.875733, 0.29088),
    float4(0.1, 0.546293, 0.884267, 0.30112),
    float4(0.1, 0.54288, 0.8928, 0.31136),
    float4(0.1, 0.539467, 0.901333, 0.3216),
    float4(0.1, 0.536053, 0.909867, 0.33184),
    float4(0.1, 0.53264, 0.9184, 0.34208),
    float4(0.1, 0.529227, 0.926933, 0.35232),
    float4(0.1, 0.525813, 0.935467, 0.36256),
    float4(0.1, 0.5224, 0.944, 0.3728),
    float4(0.1, 0.518987, 0.952533, 0.38304),
    float4(0.1, 0.515573, 0.961067, 0.39328),
    float4(0.1, 0.51216, 0.9696, 0.40352),
    float4(0.1, 0.508747, 0.978133, 0.41376),
    float4(0.1, 0.505333, 0.986667, 0.424),
    float4(0.1, 0.50192, 0.9952, 0.43424),
    float4(0.099552, 0.49776, 1, 0.44448),
    float4(0.098528, 0.49264, 1, 0.45472),
    float4(0.097504, 0.48752, 1, 0.46496),
    float4(0.09648, 0.4824, 1, 0.4752),
    float4(0.095456, 0.47728, 1, 0.48544),
    float4(0.094432, 0.47216, 1, 0.49568),
    float4(0.093408, 0.46704, 1, 0.50592),
    float4(0.092384, 0.46192, 1, 0.51616),
    float4(0.09136, 0.4568, 1, 0.5264),
    float4(0.090336, 0.45168, 1, 0.53664),
    float4(0.089312, 0.44656, 1, 0.54688),
    float4(0.088288, 0.44144, 1, 0.55712),
    float4(0.087264, 0.43632, 1, 0.56736),
    float4(0.08624, 0.4312, 1, 0.5776),
    float4(0.085216, 0.42608, 1, 0.58784),
    float4(0.084192, 0.42096, 1, 0.59808),
    float4(0.083168, 0.41584, 1, 0.60832),
    float4(0.082144, 0.41072, 1, 0.61856),
    float4(0.08112, 0.4056, 1, 0.6288),
    float4(0.080096, 0.40048, 1, 0.63904),
    float4(0.079072, 0.39536, 1, 0.64928),
    float4(0.078048, 0.39024, 1, 0.65952),
    float4(0.077024, 0.38512, 1, 0.66976),
    float4(0.076, 0.38, 1, 0.68),
    float4(0.074976, 0.37488, 1, 0.69024),
    float4(0.073952, 0.36976, 1, 0.70048),
    float4(0.072928, 0.36464, 1, 0.71072),
    float4(0.071904, 0.35952, 1, 0.72096),
    float4(0.07088, 0.3544, 1, 0.7312),
    float4(0.069856, 0.34928, 1, 0.74144),
    float4(0.068832, 0.34416, 1, 0.75168),
    float4(0.067808, 0.33904, 1, 0.76192),
    float4(0.066784, 0.33392, 1, 0.77216),
    float4(0.06576, 0.3288, 1, 0.7824),
    float4(0.064736, 0.32368, 1, 0.79264),
    float4(0.063712, 0.31856, 1, 0.80288),
    float4(0.062688, 0.31344, 1, 0.81312),
    float4(0.061664, 0.30832, 1, 0.82336),
    float4(0.06064, 0.3032, 1, 0.8336),
    float4(0.059616, 0.29808, 1, 0.84384),
    float4(0.058592, 0.29296, 1, 0.85408),
    float4(0.057568, 0.28784, 1, 0.86432),
    float4(0.056544, 0.28272, 1, 0.87456),
    float4(0.05552, 0.2776, 1, 0.8848),
    float4(0.054496, 0.27248, 1, 0.89504),
    float4(0.053472, 0.26736, 1, 0.90528),
    float4(0.052448, 0.26224, 1, 0.91552),
    float4(0.051424, 0.25712, 1, 0.92576),
    float4(0.0504, 0.252, 1, 0.936),
    float4(0.049376, 0.24688, 1, 0.94624),
    float4(0.048352, 0.24176, 1, 0.95648),
    float4(0.047328, 0.23664, 1, 0.96672),
    float4(0.046304, 0.23152, 1, 0.97696),
    float4(0.04528, 0.2264, 1, 0.9872),
    float4(0.044256, 0.22128, 1, 0.99744),
    float4(0.043232, 0.21616, 1, 1),
    float4(0.042208, 0.21104, 1, 1),
    float4(0.041184, 0.20592, 1, 1),
    float4(0.04016, 0.2008, 1, 1),
    float4(0.039136, 0.19568, 1, 1),
    float4(0.038112, 0.19056, 1, 1),
    float4(0.037088, 0.18544, 1, 1),
    float4(0.036064, 0.18032, 1, 1),
    float4(0.03504, 0.1752, 1, 1),
    float4(0.034016, 0.17008, 1, 1),
    float4(0.032992, 0.16496, 1, 1),
    float4(0.031968, 0.15984, 1, 1),
    float4(0.030944, 0.15472, 1, 1),
    float4(0.02992, 0.1496, 1, 1),
    float4(0.028896, 0.14448, 1, 1),
    float4(0.027872, 0.13936, 1, 1),
    float4(0.026848, 0.13424, 1, 1),
    float4(0.025824, 0.12912, 1, 1),
    float4(0.0248, 0.124, 1, 1),
    float4(0.023776, 0.11888, 1, 1),
    float4(0.022752, 0.11376, 1, 1),
    float4(0.021728, 0.10864, 1, 1),
    float4(0.020704, 0.10352, 1, 1),
    float4(0.01968, 0.0984, 1, 1),
    float4(0.018656, 0.09328, 1, 1),
    float4(0.017632, 0.08816, 1, 1),
    float4(0.016608, 0.08304, 1, 1),
    float4(0.015584, 0.07792, 1, 1),
    float4(0.01456, 0.0728, 1, 1),
    float4(0.013536, 0.06768, 1, 1),
    float4(0.012512, 0.06256, 1, 1),
    float4(0.011488, 0.05744, 1, 1),
    float4(0.010464, 0.05232, 1, 1),
    float4(0.00944, 0.0472, 1, 1),
    float4(0.008416, 0.04208, 1, 1),
    float4(0.007392, 0.03696, 1, 1),
    float4(0.006368, 0.03184, 1, 1),
    float4(0.005344, 0.02672, 1, 1),
    float4(0.00432, 0.0216, 1, 1),
    float4(0.003296, 0.01648, 1, 1),
    float4(0.002272, 0.01136, 1, 1),
    float4(0.001248, 0.00624, 1, 1),
    float4(0.000224, 0.00112, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 1, 1)
};
