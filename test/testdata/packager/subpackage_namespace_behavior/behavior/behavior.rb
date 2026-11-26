# typed: true

module Behavior # error: `Behavior` cannot define behavior, because packages are nested under `Behavior`
  def self.helper; end

  class Thing; end
end

Behavior.helper # error: Method `helper` does not exist on `T.class_of(Behavior)`
Behavior::Thing.new # error: Unable to resolve constant `Thing`

module Behavior::Extra
end
