# typed: true

module Outer
  def self.helper; end

  class Thing; end
end

Outer.helper
Outer::Thing
