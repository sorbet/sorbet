# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL


module M
  extend T::Sig

  # INITIAL-LABEL: @func_M.7dig_x_y
  # INITIAL: call i64 @sorbet_int_rb_hash_dig
  # INITIAL-NOT: sorbet_i_send
  # INITIAL{LITERAL}: }
  sig {params(x: T::Hash[Symbol, T.untyped]).returns(T.untyped)}
  def self.dig_x_y(x)
    x.dig(:x, :y)
  end
end

puts M.dig_x_y({x: {y: 10}})
puts M.dig_x_y({x: {z: 10}})
