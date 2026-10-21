# typed: true

module Behavior
  def self.helper; end

  class Thing; end
end

Behavior.helper
Behavior::Thing.new

module Behavior::Extra
end
