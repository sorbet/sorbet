# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Array[T.untyped])}
def keys(x)
  x.keys
end

# INITIAL-LABEL: define internal i64 @"func_Object#4keys"
# INITIAL: call i64 @sorbet_rb_hash_keys
# INITIAL{LITERAL}: }

sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Array[T.untyped])}
def values(x)
  x.values
end

# INITIAL-LABEL: define internal i64 @"func_Object#6values"
# INITIAL: call i64 @sorbet_rb_hash_values
# INITIAL{LITERAL}: }

p keys({a: 1, b: 2})
p values({a: 1, b: 2})

class HashSubclass < Hash
  def keys
    ["something else"]
  end

  def values
    ["a complete fiction"]
  end
end

p keys(HashSubclass.new)
p values(HashSubclass.new)
