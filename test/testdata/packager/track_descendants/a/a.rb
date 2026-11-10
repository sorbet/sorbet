# typed: strict

module Root::A
  class Parent
    extend T::Helpers
    track_descendants!
  end

  module Mixin
    extend T::Helpers
    track_descendants!
  end

  class Untracked; end

  NotAModule = 1

  # Registered in this package's RBI.
  class LocalChild < Parent; end
end
