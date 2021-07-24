# frozen_string_literal: true
# typed: true
# compiled: true

T::Sig::WithoutRuntime.sig(:final) {params(x: Integer, y: Integer, z: Integer).returns(Integer)}
def tarai( x, y, z )
  if x <= y
  then y
  else tarai(tarai(x-1, y, z),
             tarai(y-1, z, x),
             tarai(z-1, x, y))
  end
end

puts tarai(12, 6, 0)
