# typed: true

class A

  ##
  # ^ completion: (nothing)
  module Inner
    extend T::Sig
    ##
    # ^ completion: ##
    # ^ apply-completion: [A] item: 0
    sig {params(x: Integer, y: String).returns(NilClass)}
    def foo(x, y); end
  end
end
