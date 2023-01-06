# typed: strict

module MyPackage
  module ParentNamespace
    Const1 = 1
    Const2 = 2
    PrivateConst3 = 3
  end

  module BehaviorDefiningParentNamespace
    extend T::Sig

    sig {void}
    def behavior
    end

    Const1 = 4
  end
end
