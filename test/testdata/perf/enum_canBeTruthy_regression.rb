# typed: true
#
# This test will time out when `dropSubtypesOf` is called too frequently on
# values of type `Test`. The original case for this was with `canBeTruthy`,
# which was previously implemented in terms of `dropSubtypesOf`.

class Test
  extend T::Helpers

  abstract!
  sealed!
end

class Val0  < Test; end
class Val1  < Test; end
class Val2  < Test; end
class Val3  < Test; end
class Val4  < Test; end
class Val5  < Test; end
class Val6  < Test; end
class Val7  < Test; end
class Val8  < Test; end
class Val9  < Test; end
class Val10 < Test; end
class Val11 < Test; end
class Val12 < Test; end
class Val13 < Test; end
class Val14 < Test; end
class Val15 < Test; end
class Val16 < Test; end
class Val17 < Test; end
class Val18 < Test; end
class Val19 < Test; end
class Val20 < Test; end
class Val21 < Test; end
class Val22 < Test; end
class Val23 < Test; end
class Val24 < Test; end
class Val25 < Test; end
class Val26 < Test; end
class Val27 < Test; end
class Val28 < Test; end
class Val29 < Test; end
class Val30 < Test; end
class Val31 < Test; end
class Val32 < Test; end
class Val33 < Test; end
class Val34 < Test; end
class Val35 < Test; end
class Val36 < Test; end
class Val37 < Test; end
class Val38 < Test; end
class Val39 < Test; end
class Val40 < Test; end

class A
  extend T::Sig

  sig {params(str: String, props: T::Hash[Symbol,Symbol], x: Test).void}
  def initialize(str, props, x)
    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    if str.empty?
      raise "oh no"
    end

    props.each do |k,v|
    end

    @a1 = T.let(x, Test)
    @b1 = T.let(x, Test)
    @c1 = T.let(x, Test)
    @x1 = T.let(x, Test)
    @y1 = T.let(x, Test)
    @z1 = T.let(x, Test)
    @a2 = T.let(x, Test)
    @b2 = T.let(x, Test)
    @c2 = T.let(x, Test)
    @x2 = T.let(x, Test)
    @y2 = T.let(x, Test)
    @z2 = T.let(x, Test)
    @a3 = T.let(x, Test)
    @b3 = T.let(x, Test)
    @c3 = T.let(x, Test)
    @x3 = T.let(x, Test)
    @y3 = T.let(x, Test)
    @z3 = T.let(x, Test)
    @a4 = T.let(x, Test)
    @b4 = T.let(x, Test)
    @c4 = T.let(x, Test)
    @x4 = T.let(x, Test)
    @y4 = T.let(x, Test)
    @z4 = T.let(x, Test)
    @a5 = T.let(x, Test)
    @b5 = T.let(x, Test)
    @c5 = T.let(x, Test)
    @x5 = T.let(x, Test)
    @y5 = T.let(x, Test)
    @z5 = T.let(x, Test)
  end
end
