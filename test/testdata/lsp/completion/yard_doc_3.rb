# typed: true

class A

  ##
  # ^ completion: (nothing)
  module Inner
    extend T::Sig
    ##
    # ^ completion: ##
    # ^ apply-completion: [A] item: 0
    sig {returns(Integer)}
    def foo(); 0; end
  end
end
