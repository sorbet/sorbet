# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: OPT

module M
  extend T::Sig

  # OPT-LABEL: define internal i64 @func_M.12array_to_ary
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Array[Integer]).returns(T::Array[Integer])}
  def self.array_to_ary(x)
    x.to_ary
  end

  # OPT-LABEL: define internal i64 @func_M.12hash_to_hash
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Hash[T.untyped, T.untyped])}
  def self.hash_to_hash(x)
    x.to_hash
  end

  # OPT-LABEL: define internal i64 @func_M.12integer_to_i
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Integer).returns(Integer)}
  def self.integer_to_i(x)
    x.to_i
  end

  # OPT-LABEL: define internal i64 @func_M.14integer_to_int
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Integer).returns(Integer)}
  def self.integer_to_int(x)
    x.to_int
  end

  # OPT-LABEL: define internal i64 @func_M.13symbol_to_sym
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Symbol).returns(Symbol)}
  def self.symbol_to_sym(x)
    x.to_sym
  end

  sig {params(x: String).returns(String)}
  def self.string_to_s(x)
    x.to_s
  end

end


p M.array_to_ary([1,2,3])

p M.hash_to_hash({x: 'hi', 10 => false})

p M.integer_to_i(10)
p M.integer_to_int(20)

p M.symbol_to_sym(:foo)

class StringSubclass < String
end

p M.string_to_s("hello!")
p M.string_to_s(StringSubclass.new)
