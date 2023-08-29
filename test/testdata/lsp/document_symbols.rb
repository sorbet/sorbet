# typed: true

class A
  extend T::Sig

  sig {void}
  def initialize
    @ivar = T.let(0, Integer)
  end

  sig {returns(Integer)}
  def next
    ret = @ivar
    @ivar += 1
    ret
  end

  # no sig
  def self.bar
    puts "hey there"
  end
end

module M
  extend T::Sig

  # should not appear, since no let
  @@nope = "doesn't work"
  # should appear, since yes let
  @@greeting = T.let("hello", String)

  sig {params(x: String).returns(String)}
  def hi(x)
    "#{@@greeting}, #{x}"
  end

  sig(:final) {params(x: String).returns(String)}
  def bye(x)
    # ^^^ hover: sig(:final) { params(x: String).returns(String) }
    "goodbye, #{x}"
  end
end

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!
  sig do
    abstract
    .params(
      first: Integer,
      second: Integer,
      third: Integer,
      fourth: Integer,
      fifth: Integer,
    )
    .returns(Integer)
  end
  def sum_many(first, second, third, fourth, fifth)
  end
end

class Child < Parent
  extend T::Sig
  sig do
    override
    .params(
      first: Integer,
      second: Integer,
      third: Integer,
      fourth: Integer,
      fifth: Integer,
    )
    .returns(Integer)
  end
  def sum_many(first, second, third, fourth, fifth)
    first + second + third + fourth + fifth
  end
end

class OuterClass
  class InnerClass; end
  module InnerModule; end
end

class OuterModule
  class InnerClass; end
  module InnerModule; end
end
