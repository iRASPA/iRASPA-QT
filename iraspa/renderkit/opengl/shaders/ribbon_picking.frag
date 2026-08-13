flat in int segmentIndex;
flat in int residueIndex;

out uvec4 vFragColor;

void main()
{
  // type 3 = ribbon; z = secondary-structure segment index, w = residue index
  vFragColor = uvec4(3u, uint(structureUniforms.structureIdentifier), uint(segmentIndex), uint(residueIndex));
}
