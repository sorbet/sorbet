# frozen_string_literal: true
# typed: strict

class Flatfiles < PackageSpec
  import Opus::Flatfiles

  export Flatfiles::MyFlatfile
  export Flatfiles::MyXMLNode
end
