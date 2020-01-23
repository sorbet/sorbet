# frozen_string_literal: true
# typed: true
# compiled: true

def fooAll(arg)
  T.cast(arg, T.all(BasicObject,Kernel))
end

def fooAny1(arg)
  T.cast(arg, T.any(Integer, Float))
end

def fooAny2(arg)
  T.cast(arg, T.any(Float, Integer))
end

def fooInt(arg)
  T.cast(arg, Integer) + T.cast(arg, Integer)
end

def fooArray(arg)
  T.cast(arg, T::Array[Integer])
end

puts fooInt(2)
puts fooAny1(2)
puts fooAny2(2)
puts fooAll(2)
puts fooArray([2])
