# typed: strict

class LayerRespectingPackage < PackageSpec
  layer 'middle'
  
  import LowestLayerPackage
end
