# frozen_string_literal: true
# typed: strict

module B
  class BClass
    extend T::Sig

    sig {returns(Integer)}
    def get_one
      A::AModule.one
    end
  end

  module BModule
    extend T::Sig

    sig {returns(A::AClass)}
    def self.get_a
      A::AClass.new
    end
  end
end
