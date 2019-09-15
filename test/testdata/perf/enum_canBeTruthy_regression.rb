# typed: true
#
# This test will time out when `dropSubtypesOf` is called too frequently on
# values of type `Test`.

module Opus
  class Enum
    extend T::Generic
    def initialize(x = nil)
    end
  end
end

class Test < Opus::Enum
  include T::Props::Serializable
  Elem = type_template(fixed: self)
  Val0 = new('0')
  Val1 = new('1')
  Val2 = new('2')
  Val3 = new('3')
  Val4 = new('4')
  Val5 = new('5')
  Val6 = new('6')
  Val7 = new('7')
  Val8 = new('8')
  Val9 = new('9')
  Val10 = new('10')
  Val11 = new('11')
  Val12 = new('12')
  Val13 = new('13')
  Val14 = new('14')
  Val15 = new('15')
  Val16 = new('16')
  Val17 = new('17')
  Val18 = new('18')
  Val19 = new('19')
  Val20 = new('20')
  Val21 = new('21')
  Val22 = new('22')
  Val23 = new('23')
  Val24 = new('24')
  Val25 = new('25')
  Val26 = new('26')
  Val27 = new('27')
  Val28 = new('28')
  Val29 = new('29')
  Val30 = new('30')
  Val31 = new('31')
  Val32 = new('32')
  Val33 = new('33')
  Val34 = new('34')
  Val35 = new('35')
  Val36 = new('36')
  Val37 = new('37')
  Val38 = new('38')
  Val39 = new('39')
  Val40 = new('40')
end

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

    @metadata = T.let({x: x.serialize}, T::Hash[Symbol, T.any(String,Symbol)])

    freeze
  end
end
