# typed: true
# https://github.com/sorbet/sorbet/issues/8304
#
# Defining `Object#call` (e.g. via an anonymous `Class.new` in a test) used to
# make block bodies with nilable block parameters look unreachable, because
# LoadYieldParams called getCallArguments on the nilable type and glb'd against
# NilClass's inherited Object#call.
#
# T.any / T.all cases below exercise dropNil on secondary DispatchResult
# components. Intersection receivers use interface modules so T.all is
# inhabitable (unlike T.all of unrelated classes).
extend T::Sig

Class.new do
  # Zero-arity `call` makes NilClass's call args `[]`, which glbs to bottom with
  # `T.proc.params(a: String)`'s `[String]` when nil is not dropped first.
  def call
  end
end

sig { params(block: T.nilable(T.proc.params(a: String).void)).void }
def self.foo(&block)
  if block
    block.call("a")
  end
end

foo do |a|
  T.reveal_type(a) # error: Revealed type: `String`
end

foo

sig { params(block: T.nilable(Proc)).void }
def self.untyped_proc(&block)
end

untyped_proc do |a|
  T.reveal_type(a) # error: Revealed type: `T.untyped`
end

module NilableProcStringParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.nilable(T.proc.params(a: String).void)).void }
  def foo(&block); end
end

module NilableProcNilableStringParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.nilable(T.proc.params(a: T.nilable(String)).void)).void }
  def foo(&block); end
end

module NilableIntegerParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.nilable(T.proc.params(a: Integer).void)).void }
  def foo(&block); end
end

module NumericParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.proc.params(a: Numeric).void).void }
  def foo(&block); end
end

module StringParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.proc.params(a: String).void).void }
  def foo(&block); end
end

module IntegerParam
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.params(block: T.proc.params(a: Integer).void).void }
  def foo(&block); end
end

sig { params(x: T.any(NilableProcNilableStringParam, IntegerParam)).void }
def union_different_block_args(x)
  x.foo do |a|
    T.reveal_type(a) # error: Revealed type: `T.nilable(T.any(String, Integer))`
  end
end

sig { params(x: T.any(NilableProcNilableStringParam, StringParam)).void }
def union_same_block_args(x)
  x.foo do |a|
    T.reveal_type(a) # error: Revealed type: `T.nilable(String)`
  end
end

sig { params(x: T.all(NilableProcStringParam, StringParam)).void }
def intersection_same_block_args(x)
  x.foo do |a|
    T.reveal_type(a) # error: Revealed type: `String`
  end
end

sig { params(x: T.all(NumericParam, IntegerParam)).void }
def intersection_compatible_block_args(x)
  x.foo do |a|
    # glb(Numeric, Integer) == Integer
    T.reveal_type(a) # error: Revealed type: `Integer`
  end
end

sig { params(x: T.all(T.any(NilableProcNilableStringParam, StringParam), NilableProcNilableStringParam)).void }
def interleaved_all_any(x)
  x.foo do |a|
    T.reveal_type(a) # error: Revealed type: `T.nilable(String)`
  end
end

sig { params(x: T.any(T.all(NilableProcStringParam, StringParam), NilableIntegerParam)).void }
def interleaved_any_all(x)
  x.foo do |a|
    T.reveal_type(a) # error: Revealed type: `T.any(Integer, String)`
  end
end

sig { params(x: T.all(NilableProcStringParam, IntegerParam)).void }
def intersection_incompatible_block_args(x)
  # Even with dropNil, AND of [String] and [Integer] is bottom, so the block
  # body is unreachable. This is independent of Object#call pollution.
  x.foo do |a|
          # ^ error: This code is unreachable
    puts a
  end
end
