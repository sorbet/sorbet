# typed: true

module Behavior # error: `Behavior` can only define behavior if package `Behavior` is marked `outer_package_with_namespace_behavior!`
  def self.helper; end

  class Thing; end
end

Behavior.helper
Behavior::Thing.new

module Behavior::Extra
end
