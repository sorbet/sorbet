# typed: strict

module First
  module Second
    module Third
      module Fourth
        extend T::Generic

        module NestedModule; end
        #      ^ show-symbol: First::Second::Third::Fourth::NestedModule

        puts NestedModule
        #    ^ show-symbol: First::Second::Third::Fourth::NestedModule

        NestedConstant = 1
        # ^ show-symbol: First::Second::Third::Fourth::NestedConstant

        # TODO(jez) We should find a way to not dealias this
        NestedClassAlias = Integer
        # ^ show-symbol: Integer

        NestedTypeMember = type_member
        # ^ show-symbol: First::Second::Third::Fourth::NestedTypeMember
      end
    end

    NestedTypeAlias = T.type_alias {Integer}
    # ^ show-symbol: First::Second::NestedTypeAlias
  end
end

module Outer
  module Middle
    module Inner
      extend T::Sig
      sig {void}
      def self.foo
        # This is kind of ugly. It's possible that we want to return @foo here.
        @foo = T.let(@foo, T.nilable(String))
        #             ^ show-symbol: T.class_of(Outer::Middle::Inner)#@foo
      end

      # TODO(jez) We should probably do method calls too, but I was lazy
      foo
      # ^ show-symbol: null

      puts(:foo)
      #    ^ show-symbol: null

      sig {params(x: Integer).void}
      def bar(x)
        # TODO(jez) Does it make sense to just return the variable name here?
        Kernel.puts(x)
        #           ^ show-symbol: null
      end
    end
  end
end
