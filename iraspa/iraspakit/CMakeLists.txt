target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  iraspakitprotocols.cpp

  # projects
  documentdata.cpp
  iraspaproject.cpp
  movie.cpp
  project.cpp
  projectgroup.cpp
  projectstructure.cpp
  projecttreecontroller.cpp
  projecttreenode.cpp
  scene.cpp
  scenelist.cpp

  # objects
  crystal.cpp
  crystalcylinderprimitive.cpp
  crystalellipsoidprimitive.cpp
  crystalpolygonalprismprimitive.cpp
  cylinderprimitive.cpp
  dna.cpp
  dnaatomtreebuilder.cpp
  dnabackbone.cpp
  dnacrystal.cpp
  dnanucleotidegeometry.cpp
  dnaribbonmixin.cpp
  dnaribbonstructureeditor.cpp
  ellipsoidprimitive.cpp
  gaussiancubevolume.cpp
  gridvolume.cpp
  iraspaobject.cpp
  molecularcrystal.cpp
  molecule.cpp
  object.cpp
  polygonalprismprimitive.cpp
  primitive.cpp
  protein.cpp
  proteinaminoacidresiduereplacer.cpp
  proteinatomtreebuilder.cpp
  proteinbackbone.cpp
  proteinbspline.cpp
  proteincatmullromspline.cpp
  proteincrystal.cpp
  proteinnucleicacidmesh.cpp
  proteinribbonmesh.cpp
  proteinribbonmeshparameters.cpp
  proteinribbonmixin.cpp
  proteinribbonsecondarystructure.cpp
  proteinribbonsegmentsupport.cpp
  raspadensityvolume.cpp
  ribbonstructureeditor.cpp
  structure.cpp
  vaspdensityvolume.cpp
  vtkdensityvolume.cpp

  # interfaces
  annotationviewer.cpp
  atomstructureviewer.cpp
  atomviewer.cpp
  bondstructureviewer.cpp
  bondviewer.cpp
  displayable.cpp
  infoviewer.cpp
  primitivestructureviewer.cpp
  spacegroupviewer.cpp
  structuralpropertyviewer.cpp
  unitcellviewer.cpp
  volumetricdataviewer.cpp
)
