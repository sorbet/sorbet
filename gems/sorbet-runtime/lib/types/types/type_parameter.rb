# frozen_string_literal: true
# typed: true

module T::Types
  class TypeParameter < Base
    def initialize(name)
      raise ArgumentError.new("not a symbol: #{name}") unless name.is_a?(Symbol)
      @name = name
    end

    def valid?(obj)
      true
    end

    def subtype_of_single?(type)
      true
    end

    def name
      "T.type_parameter(:#{@name})"
    end
  end
end
