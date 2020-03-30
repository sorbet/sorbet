# frozen_string_literal: true
# typed: strong
# compiled: true

# We want to make sure that the compiler's DSL pass doesn't introduce errors at `# typed: strong`
# (because we run the DSL passes on all files, whether or not we compile them)

module Foo
  extend T::Sig

  sig {void}
  def self.bar
  end
end
