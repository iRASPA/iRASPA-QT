vec3 ribbonSelectionExpandedPosition(vec4 vertexPosition, vec4 vertexNormal, float expansionScale)
{
  vec3 localNormal = normalize(vertexNormal.xyz);
  float expansion = (structureUniforms.atomSelectionScaling - 1.0) * expansionScale;
  return vertexPosition.xyz + localNormal * expansion;
}
