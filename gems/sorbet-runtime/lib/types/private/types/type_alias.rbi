# typed: true

module T::Private::Types
  class TypeAlias < T::Types::Base
    def initialize(callable)
      @callable = T.let(callable, T.proc.void)
      @checked_level = T.let(nil, T.nilable(Symbol))
      @aliased_type = T.let(nil, T.nilable(T::Types::Base))
      @effective_aliased_type = T.let(nil, T.nilable(T::Types::Base))
    end
  end
end
