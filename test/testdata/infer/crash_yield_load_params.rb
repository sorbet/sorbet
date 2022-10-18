# typed: true

module Left
  extend T::Sig
  sig {params(blk: BasicObject).returns(NilClass)}
  #           ^^^ error: Block argument type must be either `Proc` or a `T.proc` type (and possibly nilable)
  def foo(&blk); end
end

module Up
  extend T::Sig
  sig {params(blk: BasicObject).returns(NilClass)}
  #           ^^^ error: Block argument type must be either `Proc` or a `T.proc` type (and possibly nilable)
  def foo(&blk); end
end

class Main
  extend T::Sig

  sig {params(x: T.all(Left, Up)).void}
  def foo(x)
    res = x.foo do |y|
      T.reveal_type(y) # error: `T.untyped`
    end
    res = T.reveal_type(res) # error: `NilClass`
  end
end
