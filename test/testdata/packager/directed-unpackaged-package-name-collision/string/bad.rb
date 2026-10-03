# typed: true

class String # error: File belongs to package `String` but defines a constant that does not match this namespace
  def self.from_invalid_reopening
    0
  end

  class Nested
  end

  # The lexical parent's error suppresses errors even when the namespace resets.
  class ::Consumer::Escaped
  end
end
