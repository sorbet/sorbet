# frozen_string_literal: true
# typed: true
require_relative '../../../lib/sorbet-runtime'

# Regression test for T::Utils.run_all_sig_blocks evaluation order: pending
# sig blocks must run in global declaration (FIFO) order, even when
# declarations are interleaved across multiple owners (different classes, or
# instance methods vs singleton methods of the same class). The order is
# user-visible: sig blocks can have side effects, and when several pending
# sigs are broken, it determines which error an eager boot surfaces first.

ORDER = []

class AA
  extend T::Sig

  sig { ORDER << :aa_m1; params(x: Integer).returns(Integer) }
  def m1(x)
    x
  end
end

class BB
  extend T::Sig

  sig { ORDER << :bb_m2; returns(Integer) }
  def m2
    2
  end
end

class AA
  sig { ORDER << :aa_m3; returns(Integer) }
  def m3
    3
  end

  # Singleton methods have a different owner (the singleton class) than
  # instance methods, so a registry that grouped pending sig blocks by owner
  # would run this block out of order.
  sig { ORDER << :aa_singleton_m4; returns(Integer) }
  def self.m4
    4
  end

  sig { ORDER << :aa_m5; returns(Integer) }
  def m5
    5
  end
end

T::Utils.run_all_sig_blocks
puts ORDER.join(",")
