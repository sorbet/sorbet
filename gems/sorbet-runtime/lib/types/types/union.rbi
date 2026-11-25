# typed: true

module T::Types
  class Union < Base
    def initialize(types)
      @inner_types = T.let(types, T::Array[Kernel])
      @types = T.let(types, T.nilable(T::Array[Base]))
    end

    sig { params(types: T::Array[Base]).returns(String) }
    private def type_shortcuts(types); end
  end
end
