# typed: true

class A

  ##
  # ^ completion: (nothing)
  module Inner
    extend T::Sig
    ##
    # ^ completion: ##
    # ^ apply-completion: [A] item: 0
    def foo; end
  end
end
