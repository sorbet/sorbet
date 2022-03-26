# typed: true

class A

  ##
  # ^ completion: (nothing)
  module Inner
    extend T::Sig
    ##
    # ^ completion: ##
    # ^ apply-completion: [A] item: 0
    sig {params(x: Integer, blk: T.proc.params(arg0: Integer).void).returns(NilClass)}
    def foo(x, &blk); end
  end
end
