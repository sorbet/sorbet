# typed: strict

class LayerRespectingPackage < PackageSpec
  layer 'middle'
  
  import LowestLayerPackage
  test_import HighestLayerPackage
end
