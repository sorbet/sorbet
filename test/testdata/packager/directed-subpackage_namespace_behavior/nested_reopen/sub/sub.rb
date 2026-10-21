# typed: true

module NestedReopen
  def self.from_sub; end # error: This file must only define behavior in enclosing package `NestedReopen::Sub`

  module Sub
    class Thing; end
  end
end
