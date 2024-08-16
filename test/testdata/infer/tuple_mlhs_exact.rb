# typed: true
extend T::Sig

sig { returns([Integer, String]) }
def returns_2_tuple
  [0, '']
end

sig { returns([Integer, String, Symbol]) }
def returns_3_tuple
  [0, '', :""]
end

x, = returns_2_tuple # error: Attempted to unpack 2-tuple into 1 target
x, y = returns_2_tuple
x, y, z = returns_2_tuple # error: Attempted to unpack 2-tuple into 3 targets

x, y = returns_3_tuple # error: Attempted to unpack 3-tuple into 2 targets
x, y, = returns_3_tuple # error: Attempted to unpack 3-tuple into 2 targets

xs = T::Array[[Integer, String, Symbol]].new
xs.each do |(x, y)|
  #         ^^^^^^ error: Attempted to unpack 3-tuple into 2 targets
end