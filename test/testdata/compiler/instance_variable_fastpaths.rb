# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def initialize(x)
    @x = x
  end

  def fast_get
    instance_variable_get(:@x)
  end


# Can't really test too many or too few args because those won't get past Sorbet.

  def non_constant_arg_get(sym)
    instance_variable_get(sym)
  end


  def fast_set(x)
    instance_variable_set(:@x, x)
  end


  def non_constant_arg_set(sym, value)
    instance_variable_set(sym, value)
  end


end

a = A.new(897)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

a.fast_set(76)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

a.non_constant_arg_set(:@x, 492)
a.non_constant_arg_set(:@y, 371)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

class FewArgs
  def instance_variable_get
    "nothing"
  end


  def instance_variable_set
    "nada"
  end


end

f = FewArgs.new
p f.instance_variable_get
p f.instance_variable_set

class ManyArgs
  def instance_variable_get(x, y)
    x + y
  end


  def instance_variable_set(x, y, z)
    x + y + z
  end


end

m = ManyArgs.new
p m.instance_variable_get(3, 9)
p m.instance_variable_set(1, 8, 5)
