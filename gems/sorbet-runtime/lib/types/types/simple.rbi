# typed: true

module T::Types
  class Simple < Base
    NAME_METHOD = T.let(nil, UnboundMethod)

    def initialize(raw_type)
      @raw_type = T.let(raw_type, Module)
      @name = T.let(nil, T.nilable(String))
      @nilable = T.let(nil, T.nilable(T::Private::Types::SimplePairUnion))
    end

    sig { params(obj: Kernel).returns(String) }
    private def error_message(obj); end

    module Private
      module Pool
        CACHE_FROZEN_OBJECTS = T.let(true, T::Boolean)

        @cache = T.let(nil, ObjectSpace::WeakMap)

        sig { params(mod: Module).returns(Base) }
        def self.type_for_module(mod)
        end
      end
    end
  end
end
