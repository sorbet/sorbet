# typed: true

module Behavior # error: `Behavior` can only define behavior if package `Behavior` is marked `outer_package_with_namespace_behavior!`
  def self.helper; end

  class Thing; end
end

Behavior.helper # error: Method `helper` does not exist on `T.class_of(Behavior)`
Behavior::Thing.new # error: Unable to resolve constant `Thing`

module Behavior::Extra
end
