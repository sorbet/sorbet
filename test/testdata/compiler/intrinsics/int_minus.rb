# frozen_string_literal: true
# typed: true
# compiled: true

# FIXNUM, FIXNUM
puts(1 - 1)

# FIXNUM, T_BIGNUM
puts(1 - T.unsafe(100 ** 100))

# FIXNUM, T_FLOAT
puts(1 - T.unsafe(1.0))
puts(1 - T.unsafe(Float::NAN))
puts(1 - T.unsafe(Float::INFINITY))
puts(1 - T.unsafe(-Float::INFINITY))

# T_BIGNUM, *
puts(T.cast(100**100, Integer) - 1)
