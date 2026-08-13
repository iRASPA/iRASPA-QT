in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;
in vec2 vertexPad;
in vec2 vertexStripeST;

flat out int segmentIndex;
flat out int residueIndex;

void main()
{
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * vertexPosition;
  segmentIndex = int(vertexNormal.w);
  residueIndex = int(vertexPad.y);
}
