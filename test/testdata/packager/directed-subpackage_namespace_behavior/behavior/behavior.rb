# typed: true

module Behavior # error: `Behavior` cannot define behavior, because packages are nested under `Behavior`
  def self.helper; end

  class Thing; end
end

Behavior.helper
Behavior::Thing.new

module Behavior::Extra
end
