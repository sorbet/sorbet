# frozen_string_literal: true
# typed: true
# compiled: true

# FIXNUM, FIXNUM
puts(1 + 1)

# FIXNUM, T_BIGNUM
puts(1 + T.unsafe(100 ** 100))

# FIXNUM, T_FLOAT
puts(1 + T.unsafe(1.0))

# T_BIGNUM, *
puts(T.cast(100*100, Integer) + 1)
