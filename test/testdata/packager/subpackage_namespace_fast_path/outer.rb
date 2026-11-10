# typed: true

module Outer # error: `Outer` cannot define behavior, because packages are nested under `Outer`
  def self.helper; end

  class Thing; end
end

Outer.helper
Outer::Thing
