# typed: true

module T::Types
  class TypedEnumerable < Base
    def initialize(type)
      @inner_type = T.let(type, T.anything)
      @type = T.let(nil, T.nilable(T::Types::Base))
    end
  end
end
