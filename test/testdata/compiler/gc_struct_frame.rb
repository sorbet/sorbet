# frozen_string_literal: true
# frozen_string_literal: true
# typed: true
# compiled: true

# This test stresses a bug that we found in sorbet_allocateRubyStackFrames
# where at large stack depths, the Ruby VM would wake up and try to free
# iseq's that we'd allocated.

module Opus; end
module Opus::Utils; end
module Opus::Utils::DeepFreeze
  extend T::Sig

  sig {params(n: Integer).returns(T::Array[T.untyped])}
  def self.generate_deep(n)
    o = T.let(nil, T.untyped)
    n.times do
      o = Struct.new(:o, :p).new(o, "p".dup)
    end
    o
  end

  def self.main
    100.times.map do
      Opus::Utils::DeepFreeze.generate_deep(1)
    end

    0
  end
end

Opus::Utils::DeepFreeze.main
