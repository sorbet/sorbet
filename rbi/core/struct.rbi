# typed: __STDLIB_INTERNAL

class Struct < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
        keyword_init: T::Boolean,
    )
    .void
  end
  def initialize(arg0, *arg1, keyword_init: false); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  sig {returns(T::Array[Symbol])}
  def self.members; end

  sig {params(args: T.untyped).returns(Struct)}
  def new(*args); end
end
