# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  const :array_int, T::Array[Integer], default: []

  const :hash_symbol_int, T::Hash[Symbol, Integer], default: {}
end

a1 = A.new
a1.array_int << 42
p a1.array_int
a1.hash_symbol_int[:foo] = 1243
p a1.hash_symbol_int

# This tests that the default [] / {} is deep copied (not shared)
a2 = A.new
p a2.array_int
p a2.hash_symbol_int
