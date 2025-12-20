# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: OPT

module M
  extend T::Sig

  # OPT-LABEL: define internal {{.*}}i64 @func_M.12array_to_ary
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Array[Integer]).returns(T::Array[Integer])}
  def self.array_to_ary(x)
    x.to_ary
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.12hash_to_hash
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Hash[T.untyped, T.untyped])}
  def self.hash_to_hash(x)
    x.to_hash
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.9hash_to_h
  # OPT: sorbet_rb_hash_to_h
  # OPT: sorbet_callFuncWithCache
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Hash[T.untyped, T.untyped])}
  def self.hash_to_h(x)
    x.to_h
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.19hash_to_h_withBlock
  # OPT: sorbet_callFuncWithCache
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT-CHECK: call void @rb_hash_foreach(i64 %rawArg_x,a.*func_M.19hash_to_h_withBlock\$block_1
  # OPT{LITERAL}: }
  sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Hash[T.untyped, T.untyped])}
  def self.hash_to_h_withBlock(x)
    x.to_h {|k,v| [v,k]}
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.12integer_to_i
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Integer).returns(Integer)}
  def self.integer_to_i(x)
    x.to_i
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.14integer_to_int
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Integer).returns(Integer)}
  def self.integer_to_int(x)
    x.to_int
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.13symbol_to_sym
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: Symbol).returns(Symbol)}
  def self.symbol_to_sym(x)
    x.to_sym
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.11string_to_s
  # OPT: sorbet_rb_str_to_s
  # OPT: sorbet_callFuncWithCache
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: String).returns(String)}
  def self.string_to_s(x)
    x.to_s
  end

  # OPT-LABEL: define internal {{.*}}i64 @func_M.10array_to_a
  # OPT: sorbet_rb_ary_to_a
  # OPT: sorbet_callFuncWithCache
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Array[T.untyped]).returns(T::Array[T.untyped])}
  def self.array_to_a(x)
    x.to_a
  end

end


p M.array_to_ary([1,2,3])

p M.hash_to_hash({x: 'hi', 10 => false})

class HashSubclass < Hash
end

p M.hash_to_h({x: 'hi', 10 => false})
p M.hash_to_h({x: 'hi', 10 => false}).class
p M.hash_to_h(HashSubclass.new)
p M.hash_to_h(HashSubclass.new).class

p M.hash_to_h_withBlock({x: 'hi', 10 => false})
p M.hash_to_h_withBlock({x: 'hi', 10 => false}).class
p M.hash_to_h_withBlock(HashSubclass.new)
p M.hash_to_h_withBlock(HashSubclass.new).class

p M.integer_to_i(10)
p M.integer_to_int(20)

p M.symbol_to_sym(:foo)

class StringSubclass < String
end

class OtherStringSubclass < String
  def to_s
    'other string subclass!'
  end
end

p M.string_to_s("hello!")
p M.string_to_s(StringSubclass.new)
p M.string_to_s(OtherStringSubclass.new)

class ArraySubclass < Array
end

class OtherArraySubclass < Array
  def to_a
    [1,2,3,'other','array','subclass']
  end
end

p M.array_to_a([1,2,3,4,5])
p M.array_to_a(ArraySubclass.new)
p M.array_to_a(OtherArraySubclass.new)
