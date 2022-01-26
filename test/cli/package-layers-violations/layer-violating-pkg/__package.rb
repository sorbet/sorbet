# typed: strict

class LayerViolatingPackage < PackageSpec
  layer 'middle'

  import LayerRespectingPackage
  import HighestLayerPackage
  import LowestLayerPackage
end
