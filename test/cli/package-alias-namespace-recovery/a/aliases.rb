# typed: true

module A
  class Thing; end
  Same = Thing
  Foreign = B::Thing
  Chained = Foreign
  ModuleAlias = B::Mixin
  Unpackaged = String
end
