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
    "goodbye, #{x}"
  end
end

class BigDecimal
end
