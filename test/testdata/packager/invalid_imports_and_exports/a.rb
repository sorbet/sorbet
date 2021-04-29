# frozen_string_literal: true
# typed: strict

module A
  REFERENCE = ASecondClass

  class ASecondClass

  end

  class AClass
    extend T::Sig

    sig {returns(AClass)}
    def get_a
      B::BModule.get_a
    end
  end


  module AModule
    extend T::Sig

    sig {returns(Integer)}
    def self.one
      1
    end
  end
end
