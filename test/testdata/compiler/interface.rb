# frozen_string_literal: true
# typed: strict
# compiled: true

module Base
  extend T::Sig
  extend T::Helpers
  interface!
  sig {abstract.returns(Integer)}
  def no_args; end

  sig {abstract.params(x: Integer).returns(Integer)}
  def one_arg(x); end

  sig {abstract.params(x: Integer).returns(Integer)}
  def keyword_arg(x:); end

  sig {abstract.params(x: Integer, y: Integer, z: Integer, other: Integer).returns(Integer)}
  def mixed(x, y, z:, other:); end
end

module Impl
  include Kernel
  extend T::Sig
  sig {returns(Integer)}
  def no_args
    17
  end

  sig {params(x: Integer).returns(Integer)}
  def one_arg(x)
    x
  end

  sig {params(x: Integer).returns(Integer)}
  def keyword_arg(x:)
    x
  end

  sig {params(x: Integer, y: Integer, z: Integer, other: Integer).returns(Integer)}
  def mixed(x, y, z:, other:)
    (x + y + z) * other
  end
end

class A
  include Impl
  include Base
end

class B
  include Base
  include Impl
end

class Test
  extend T::Sig

  sig {params(base: Base).void}
  def self.test(base)
    puts base.no_args
    puts base.one_arg 10
    puts base.keyword_arg(x: 10)
    puts base.mixed(1, 2, other: 10, z: 20)
  end
end

Test.test A.new
Test.test B.new
