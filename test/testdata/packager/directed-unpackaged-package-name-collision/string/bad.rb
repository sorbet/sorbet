# typed: true

class String # error: File belongs to package `String` but defines a constant that does not match this namespace
  def self.from_invalid_reopening
    0
  end

  class Nested
  end

  # Namespace resets report their own violation even inside an invalid definition.
  class ::Consumer::Escaped # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  end
end
