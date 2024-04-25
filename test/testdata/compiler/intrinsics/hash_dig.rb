# frozen_string_literal: true
# typed: true
# compiled: true


module M
  extend T::Sig

  sig {params(x: T::Hash[Symbol, T.untyped]).returns(T.untyped)}
  def self.dig_x_y(x)
    x.dig(:x, :y)
  end
end

puts M.dig_x_y({x: {y: 10}})
puts M.dig_x_y({x: {z: 10}})
