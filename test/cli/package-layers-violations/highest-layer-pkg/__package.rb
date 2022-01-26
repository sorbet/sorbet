# typed: strict

class HighestLayerPackage < PackageSpec
  layer 'highest'

  import LayerRespectingPackage
  import LowestLayerPackage
end
