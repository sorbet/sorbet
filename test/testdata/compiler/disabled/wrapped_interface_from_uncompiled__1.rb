# frozen_string_literal: true
# typed: strict
# compiled: true

require_relative './wrapped_interface_from_uncompiled__2'

class Test
  extend T::Sig

  sig {params(base: Base).void}
  def self.test1(base) ; end

  sig {params(base: Base).returns(Base)}
  def self.test2(base)
    base
  end

  sig {returns(Base)}
  def self.test3
    make_wrapped_a
  end
end

a = make_wrapped_a
Test.test1(a)
Test.test2(a)
Test.test3
