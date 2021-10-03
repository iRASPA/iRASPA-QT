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
#include "opengldensityvolumeshader.h"
#include "unitcubegeometry.h"
#include "spheregeometry.h"
#include "cubegeometry.h"
#include <qmath.h>
#include <math.h>
#include <cmath>


OpenGLDensityVolumeShader::OpenGLDensityVolumeShader()
{

}


void OpenGLDensityVolumeShader::deleteBuffers()
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

void OpenGLDensityVolumeShader::generateBuffers()
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

void OpenGLDensityVolumeShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}



void OpenGLDensityVolumeShader::paintGL(GLuint structureUniformBuffer)
{
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDepthMask(GL_FALSE);
  //glDepthFunc(GL_ALWAYS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (std::shared_ptr<RKRenderDensityVolumeSource> object = std::dynamic_pointer_cast<RKRenderDensityVolumeSource>(_renderStructures[i][j]))
      {
        if(_renderStructures[i][j]->isVisible())
        {
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_3D, _volumeTextures[i][j]);
          check_gl_error();
          glUniform1i(_volumeTextureUniformLocation, 0);
          check_gl_error();

          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_1D, _transferFunctionTexture);
          check_gl_error();
          glUniform1i(_transferFunctionUniformLocation, 1);
          check_gl_error();

          GLfloat m_stepLength=0.0005;

          glUniform1f(_stepLengthUniformLocation, m_stepLength);
          check_gl_error();

          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));
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


void OpenGLDensityVolumeShader::reloadData()
{
  initializeVertexArrayObject();
  initializeTransferFunctionTexture();
}

void OpenGLDensityVolumeShader::initializeNoiseTexture()
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  const int width = viewport[2];
  const int height = viewport[3];

  qDebug() << "width, heigth" << width << height;

  std::srand(std::time(NULL));
  unsigned char noise[width * height];

  for (unsigned char *p = noise; p <= noise + width * height; ++p) {
      *p = std::rand() % 256;
  }

  glDeleteTextures(1, &_noiseTexture);
  check_gl_error();
  glGenTextures(1, &_noiseTexture);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, _noiseTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, noise);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLDensityVolumeShader::initializeTransferFunctionTexture()
{
  glDeleteTextures(1, &_transferFunctionTexture);
  check_gl_error();
  glGenTextures(1, &_transferFunctionTexture);
  check_gl_error();
  glBindTexture(GL_TEXTURE_1D, _transferFunctionTexture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, transferFunction.size(), 0, GL_RGBA, GL_FLOAT, transferFunction.data());
  check_gl_error();
  glBindTexture(GL_TEXTURE_1D, 0);
}

void OpenGLDensityVolumeShader::initializeVertexArrayObject()
{
    CubeGeometry sphere = CubeGeometry();

    for(size_t i=0;i<_renderStructures.size();i++)
    {
      for(size_t j=0;j<_renderStructures[i].size();j++)
      {
        if (std::shared_ptr<RKRenderDensityVolumeSource> object = std::dynamic_pointer_cast<RKRenderDensityVolumeSource>(_renderStructures[i][j]))
        {
          glBindTexture(GL_TEXTURE_3D, _volumeTextures[i][j]);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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

void OpenGLDensityVolumeShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLDensityVolumeShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLDensityVolumeShader::_fragmentShaderSource.c_str());

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

void OpenGLDensityVolumeShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
}

void OpenGLDensityVolumeShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
}

void OpenGLDensityVolumeShader::initializeLightUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
}


// Based on: page 168 "Real-Time Volume Graphics", Klaus Engels et al.

const std::string  OpenGLDensityVolumeShader::_vertexShaderSource =
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
  vec4 pos = structureUniforms.boxMatrix * vertexPosition;
  vs_out.position = pos.xyz;
  vs_out.UV =  vertexPosition.xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vec4 dir = lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w;
  vs_out.L = (inverse(frameUniforms.viewMatrix) * inverse(frameUniforms.normalMatrix) * inverse(structureUniforms.boxMatrix) * vec4(dir.xyz,0.0)).xyz;

  vs_out.clipPosition = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo";

const std::string  OpenGLDensityVolumeShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
out vec4 vFragColor;
// Input from vertex shader

in VS_OUT
{
  smooth vec3 UV;
  vec3 position;
  smooth vec3 L;
  smooth vec4 clipPosition;
} fs_in;

uniform float step_length;

uniform sampler3D volume;
uniform sampler1D transferFunction;

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

const int numSamples = 5000;

void main()
{
  vec3 direction = normalize(fs_in.position.xyz - frameUniforms.cameraPosition.xyz);
  vec4 dir = vec4(direction.x,direction.y,direction.z,0.0);
  //dir = vec4(0,0,-1,0);
  vec3 ray_direction = (inverse(structureUniforms.boxMatrix) * dir).xyz;
  //vec3 ray_direction = dir.xyz;

  vec3 ray_origin = fs_in.UV;

  float t_0, t_1;
  Ray casting_ray = Ray(ray_origin, ray_direction);
  AABB bounding_box = AABB(vec3(1.0,1.0,1.0), vec3(0.0,0.0,0.0));
  ray_box_intersection(casting_ray, bounding_box, t_0, t_1);

  vec3 ray_start = ray_origin + ray_direction * t_0;
  vec3 ray_stop = ray_origin + ray_direction * t_1;

  vec3 ray = ray_stop - ray_start;
  float ray_length = length(ray);
  vec3 step_vector = step_length * ray / ray_length;

  vec4 colour = vec4(0.0,0.0,0.0,0.0);
  vec3 position = ray_start;
  for (int i=0; i < numSamples && ray_length > 0 && colour.a < 1.0; i++)
  {
    vec4 values = texture(volume,position);
    vec3 normal = normalize(values.gba);

    //vec4 c = colour_transfer(values.r);
    vec4 c = texture(transferFunction,values.r);

    // Alpha-blending
    vec3 R = reflect(-direction, normal);
    vec3 ambient = vec3(0.3,0.3,0.3);
    vec3 diffuse = vec3(max(abs(dot(normal, direction)),0.0));
    vec3 specular = vec3(pow(max(dot(R, direction), 0.0), 0.4));

    colour.rgb = c.a * (ambient+diffuse+specular) * c.rgb + (1 - c.a) * colour.a * colour.rgb;
    colour.a = c.a + (1 - c.a) * colour.a;

    position = position + step_vector;
    ray_length -= step_length;
  }
  mat4 m = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * structureUniforms.boxMatrix;
  vec4 clipPosition = m * vec4(position,1.0);
  gl_FragDepth = 0.5*(clipPosition.z / clipPosition.w)+0.5;

  vFragColor.rgb = 1.0 - exp2(-colour.rgb * 1.5);
  vFragColor.a=colour.a;
}
)foo";

std::array<float4, 256> OpenGLDensityVolumeShader::transferFunction
{
    float4(0.5, 0.2, 0.2, 0),
    float4(0.521254, 0.204251, 0.204251, 0),
    float4(0.54259, 0.208518, 0.208518, 0),
    float4(0.563927, 0.212785, 0.212785, 0),
    float4(0.585264, 0.217053, 0.217053, 0),
    float4(0.606601, 0.22132, 0.22132, 0),
    float4(0.627938, 0.225588, 0.225588, 0),
    float4(0.649275, 0.229855, 0.229855, 0),
    float4(0.670612, 0.234122, 0.234122, 0),
    float4(0.691949, 0.23839, 0.23839, 0),
    float4(0.713286, 0.242657, 0.242657, 0),
    float4(0.734622, 0.246924, 0.246924, 0),
    float4(0.755959, 0.251192, 0.251192, 0),
    float4(0.777296, 0.255459, 0.255459, 0),
    float4(0.798633, 0.259727, 0.259727, 0),
    float4(0.81997, 0.263994, 0.263994, 0),
    float4(0.841307, 0.268261, 0.268261, 0),
    float4(0.862644, 0.272529, 0.272529, 0),
    float4(0.883981, 0.276796, 0.276796, 0),
    float4(0.905318, 0.281064, 0.281064, 0),
    float4(0.926654, 0.285331, 0.285331, 0),
    float4(0.947991, 0.289598, 0.289598, 0),
    float4(0.969328, 0.293866, 0.293866, 0),
    float4(0.990665, 0.298133, 0.298133, 0),
    float4(1, 0.303927, 0.3, 0),
    float4(1, 0.310909, 0.3, 0),
    float4(1, 0.317891, 0.3, 0),
    float4(1, 0.324873, 0.3, 0),
    float4(1, 0.331855, 0.3, 0),
    float4(1, 0.338836, 0.3, 0),
    float4(1, 0.345818, 0.3, 0),
    float4(1, 0.3528, 0.3, 0),
    float4(1, 0.359782, 0.3, 0),
    float4(1, 0.366764, 0.3, 0),
    float4(1, 0.373745, 0.3, 0),
    float4(1, 0.380727, 0.3, 0),
    float4(1, 0.387709, 0.3, 0),
    float4(1, 0.394691, 0.3, 0),
    float4(1, 0.401673, 0.3, 0),
    float4(1, 0.408655, 0.3, 0),
    float4(1, 0.415636, 0.3, 0.0288),
    float4(1, 0.422618, 0.3, 0.05952),
    float4(1, 0.4296, 0.3, 0.09024),
    float4(1, 0.436582, 0.3, 0.12096),
    float4(1, 0.443564, 0.3, 0.15168),
    float4(1, 0.450545, 0.3, 0.1824),
    float4(1, 0.457527, 0.3, 0.21312),
    float4(1, 0.464509, 0.3, 0.24384),
    float4(1, 0.471491, 0.3, 0.27456),
    float4(1, 0.478473, 0.3, 0.30528),
    float4(1, 0.485455, 0.3, 0.336),
    float4(1, 0.492436, 0.3, 0.36672),
    float4(1, 0.499418, 0.3, 0.39744),
    float4(1, 0.5064, 0.3, 0.42816),
    float4(1, 0.513382, 0.3, 0.45888),
    float4(1, 0.520364, 0.3, 0.4896),
    float4(1, 0.527345, 0.3, 0.52032),
    float4(1, 0.534327, 0.3, 0.55104),
    float4(1, 0.541309, 0.3, 0.58176),
    float4(1, 0.548291, 0.3, 0.6),
    float4(1, 0.555273, 0.3, 0.6),
    float4(1, 0.562255, 0.3, 0.6),
    float4(1, 0.569236, 0.3, 0.6),
    float4(1, 0.576218, 0.3, 0.6),
    float4(1, 0.5832, 0.3, 0.6),
    float4(1, 0.590182, 0.3, 0.6),
    float4(1, 0.597164, 0.3, 0.6),
    float4(1, 0.605527, 0.295855, 0.6),
    float4(1, 0.614836, 0.288873, 0.6),
    float4(1, 0.624145, 0.281891, 0.6),
    float4(1, 0.633455, 0.274909, 0.6),
    float4(1, 0.642764, 0.267927, 0.6),
    float4(1, 0.652073, 0.260945, 0.6),
    float4(1, 0.661382, 0.253964, 0.6),
    float4(1, 0.670691, 0.246982, 0.6),
    float4(1, 0.68, 0.24, 0.6),
    float4(1, 0.689309, 0.233018, 0.6),
    float4(1, 0.698618, 0.226036, 0.6),
    float4(1, 0.707927, 0.219055, 0.6),
    float4(1, 0.717236, 0.212073, 0.60672),
    float4(1, 0.726545, 0.205091, 0.6144),
    float4(1, 0.735855, 0.198109, 0.62208),
    float4(1, 0.745164, 0.191127, 0.62976),
    float4(1, 0.754473, 0.184145, 0.63744),
    float4(1, 0.763782, 0.177164, 0.64512),
    float4(1, 0.773091, 0.170182, 0.6528),
    float4(1, 0.7824, 0.1632, 0.66048),
    float4(1, 0.791709, 0.156218, 0.66816),
    float4(1, 0.801018, 0.149236, 0.67584),
    float4(1, 0.810327, 0.142255, 0.68352),
    float4(1, 0.819636, 0.135273, 0.6912),
    float4(1, 0.828945, 0.128291, 0.69888),
    float4(1, 0.838255, 0.121309, 0.70656),
    float4(1, 0.847564, 0.114327, 0.71424),
    float4(1, 0.856873, 0.107345, 0.72192),
    float4(1, 0.866182, 0.100364, 0.7296),
    float4(1, 0.875491, 0.0933818, 0.73728),
    float4(1, 0.8848, 0.0864, 0.74496),
    float4(1, 0.894109, 0.0794182, 0.75264),
    float4(1, 0.903418, 0.0724364, 0.76032),
    float4(1, 0.912727, 0.0654545, 0.768),
    float4(1, 0.922036, 0.0584727, 0.77568),
    float4(1, 0.931345, 0.0514909, 0.78336),
    float4(1, 0.940655, 0.0445091, 0.79104),
    float4(1, 0.949964, 0.0375273, 0.79872),
    float4(1, 0.959273, 0.0305455, 0.8064),
    float4(1, 0.968582, 0.0235636, 0.81408),
    float4(1, 0.977891, 0.0165818, 0.82176),
    float4(1, 0.9872, 0.0096, 0.82944),
    float4(1, 0.996509, 0.00261818, 0.83712),
    float4(0.904, 1, 0.096, 0.8448),
    float4(0.7504, 1, 0.2496, 0.85248),
    float4(0.5968, 1, 0.4032, 0.86016),
    float4(0.4432, 1, 0.5568, 0.86784),
    float4(0.3816, 0.9448, 0.5816, 0.87552),
    float4(0.356, 0.868, 0.556, 0.8832),
    float4(0.3304, 0.7912, 0.5304, 0.89088),
    float4(0.3048, 0.7144, 0.5048, 0.89856),
    float4(0.3, 0.698829, 0.502927, 0.900693),
    float4(0.3, 0.697389, 0.506529, 0.901547),
    float4(0.3, 0.695948, 0.510131, 0.9024),
    float4(0.3, 0.694507, 0.513733, 0.903253),
    float4(0.3, 0.693066, 0.517335, 0.904107),
    float4(0.3, 0.691625, 0.520937, 0.90496),
    float4(0.3, 0.690185, 0.524538, 0.905813),
    float4(0.3, 0.688744, 0.52814, 0.906667),
    float4(0.3, 0.687303, 0.531742, 0.90752),
    float4(0.3, 0.685862, 0.535344, 0.908373),
    float4(0.3, 0.684421, 0.538946, 0.909227),
    float4(0.3, 0.682981, 0.542548, 0.91008),
    float4(0.3, 0.68154, 0.54615, 0.910933),
    float4(0.3, 0.680099, 0.549752, 0.911787),
    float4(0.3, 0.678658, 0.553354, 0.91264),
    float4(0.3, 0.677217, 0.556956, 0.913493),
    float4(0.3, 0.675777, 0.560558, 0.914347),
    float4(0.3, 0.674336, 0.56416, 0.9152),
    float4(0.3, 0.672895, 0.567762, 0.916053),
    float4(0.3, 0.671454, 0.571364, 0.916907),
    float4(0.3, 0.670014, 0.574966, 0.91776),
    float4(0.3, 0.668573, 0.578568, 0.918613),
    float4(0.3, 0.667132, 0.58217, 0.919467),
    float4(0.3, 0.665691, 0.585772, 0.92032),
    float4(0.3, 0.66425, 0.589374, 0.921173),
    float4(0.3, 0.66281, 0.592976, 0.922027),
    float4(0.3, 0.661369, 0.596578, 0.92288),
    float4(0.3, 0.659928, 0.60018, 0.923733),
    float4(0.3, 0.658487, 0.603782, 0.924587),
    float4(0.3, 0.657046, 0.607384, 0.92544),
    float4(0.3, 0.655606, 0.610986, 0.926293),
    float4(0.3, 0.654165, 0.614588, 0.927147),
    float4(0.3, 0.652724, 0.61819, 0.928),
    float4(0.3, 0.651283, 0.621792, 0.928853),
    float4(0.3, 0.649842, 0.625394, 0.929707),
    float4(0.3, 0.648402, 0.628996, 0.93056),
    float4(0.3, 0.646961, 0.632598, 0.931413),
    float4(0.3, 0.64552, 0.6362, 0.932267),
    float4(0.3, 0.644079, 0.639802, 0.93312),
    float4(0.3, 0.642638, 0.643404, 0.933973),
    float4(0.3, 0.641198, 0.647006, 0.934827),
    float4(0.3, 0.639757, 0.650608, 0.93568),
    float4(0.3, 0.638316, 0.65421, 0.936533),
    float4(0.3, 0.636875, 0.657812, 0.937387),
    float4(0.3, 0.635434, 0.661414, 0.93824),
    float4(0.3, 0.633994, 0.665016, 0.939093),
    float4(0.3, 0.632553, 0.668618, 0.939947),
    float4(0.3, 0.631112, 0.67222, 0.9408),
    float4(0.3, 0.629671, 0.675822, 0.941653),
    float4(0.3, 0.628231, 0.679424, 0.942507),
    float4(0.3, 0.62679, 0.683026, 0.94336),
    float4(0.3, 0.625349, 0.686628, 0.944213),
    float4(0.3, 0.623908, 0.69023, 0.945067),
    float4(0.3, 0.622467, 0.693832, 0.94592),
    float4(0.3, 0.621027, 0.697434, 0.946773),
    float4(0.3, 0.619586, 0.701036, 0.947627),
    float4(0.3, 0.618145, 0.704638, 0.94848),
    float4(0.3, 0.616704, 0.70824, 0.949333),
    float4(0.3, 0.615263, 0.711842, 0.950187),
    float4(0.3, 0.613823, 0.715443, 0.95104),
    float4(0.3, 0.612382, 0.719045, 0.951893),
    float4(0.3, 0.610941, 0.722647, 0.952747),
    float4(0.3, 0.6095, 0.726249, 0.9536),
    float4(0.3, 0.608059, 0.729851, 0.954453),
    float4(0.3, 0.606619, 0.733453, 0.955307),
    float4(0.3, 0.605178, 0.737055, 0.95616),
    float4(0.3, 0.603737, 0.740657, 0.957013),
    float4(0.3, 0.602296, 0.744259, 0.957867),
    float4(0.3, 0.600855, 0.747861, 0.95872),
    float4(0.3, 0.599415, 0.751463, 0.959573),
    float4(0.3, 0.597974, 0.755065, 0.960427),
    float4(0.3, 0.596533, 0.758667, 0.96128),
    float4(0.3, 0.595092, 0.762269, 0.962133),
    float4(0.3, 0.593652, 0.765871, 0.962987),
    float4(0.3, 0.592211, 0.769473, 0.96384),
    float4(0.3, 0.59077, 0.773075, 0.964693),
    float4(0.3, 0.589329, 0.776677, 0.965547),
    float4(0.3, 0.587888, 0.780279, 0.9664),
    float4(0.3, 0.586448, 0.783881, 0.967253),
    float4(0.3, 0.585007, 0.787483, 0.968107),
    float4(0.3, 0.583566, 0.791085, 0.96896),
    float4(0.3, 0.582125, 0.794687, 0.969813),
    float4(0.3, 0.580684, 0.798289, 0.970667),
    float4(0.3, 0.579244, 0.801891, 0.97152),
    float4(0.3, 0.577803, 0.805493, 0.972373),
    float4(0.3, 0.576362, 0.809095, 0.973227),
    float4(0.3, 0.574921, 0.812697, 0.97408),
    float4(0.3, 0.57348, 0.816299, 0.974933),
    float4(0.3, 0.57204, 0.819901, 0.975787),
    float4(0.3, 0.570599, 0.823503, 0.97664),
    float4(0.3, 0.569158, 0.827105, 0.977493),
    float4(0.3, 0.567717, 0.830707, 0.978347),
    float4(0.3, 0.566276, 0.834309, 0.9792),
    float4(0.3, 0.564836, 0.837911, 0.980053),
    float4(0.3, 0.563395, 0.841513, 0.980907),
    float4(0.3, 0.561954, 0.845115, 0.98176),
    float4(0.3, 0.560513, 0.848717, 0.982613),
    float4(0.3, 0.559072, 0.852319, 0.983467),
    float4(0.3, 0.557632, 0.855921, 0.98432),
    float4(0.3, 0.556191, 0.859523, 0.985173),
    float4(0.3, 0.55475, 0.863125, 0.986027),
    float4(0.3, 0.553309, 0.866727, 0.98688),
    float4(0.3, 0.551869, 0.870329, 0.987733),
    float4(0.3, 0.550428, 0.873931, 0.988587),
    float4(0.3, 0.548987, 0.877533, 0.98944),
    float4(0.3, 0.547546, 0.881135, 0.990293),
    float4(0.3, 0.546105, 0.884737, 0.991147),
    float4(0.3, 0.544665, 0.888339, 0.992),
    float4(0.3, 0.543224, 0.891941, 0.992853),
    float4(0.3, 0.541783, 0.895543, 0.993707),
    float4(0.3, 0.540342, 0.899145, 0.99456),
    float4(0.3, 0.538901, 0.902747, 0.995413),
    float4(0.3, 0.537461, 0.906348, 0.996267),
    float4(0.3, 0.53602, 0.90995, 0.99712),
    float4(0.3, 0.534579, 0.913552, 0.997973),
    float4(0.3, 0.533138, 0.917154, 0.998827),
    float4(0.3, 0.531697, 0.920756, 0.99968),
    float4(0.3, 0.530257, 0.924358, 0.968),
    float4(0.3, 0.528816, 0.92796, 0.9168),
    float4(0.3, 0.527375, 0.931562, 0.8656),
    float4(0.3, 0.525934, 0.935164, 0.8144),
    float4(0.3, 0.524493, 0.938766, 0.7632),
    float4(0.3, 0.523053, 0.942368, 0.712),
    float4(0.3, 0.521612, 0.94597, 0.6608),
    float4(0.3, 0.520171, 0.949572, 0.6096),
    float4(0.3, 0.51873, 0.953174, 0.5584),
    float4(0.3, 0.51729, 0.956776, 0.5072),
    float4(0.3, 0.515849, 0.960378, 0.456),
    float4(0.3, 0.514408, 0.96398, 0.4048),
    float4(0.3, 0.512967, 0.967582, 0.3536),
    float4(0.3, 0.511526, 0.971184, 0.3024),
    float4(0.3, 0.510086, 0.974786, 0.2512),
    float4(0.3, 0.508645, 0.978388, 0.2),
    float4(0.3, 0.507204, 0.98199, 0.1488),
    float4(0.3, 0.505763, 0.985592, 0.0976),
    float4(0.3, 0.504322, 0.989194, 0.0464),
    float4(0.3, 0.502882, 0.992796, 0),
    float4(0.3, 0.501441, 0.996398, 0)
};
