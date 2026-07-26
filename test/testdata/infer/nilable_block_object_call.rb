# typed: true
# https://github.com/sorbet/sorbet/issues/8304
#
# Defining `Object#call` (e.g. via an anonymous `Class.new` in a test) used to
# make block bodies with nilable block parameters look unreachable, because
# LoadYieldParams called getCallArguments on the nilable type and glb'd against
# NilClass's inherited Object#call.
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
