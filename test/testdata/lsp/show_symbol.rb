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

        NestedClassAlias = Integer
        # ^ show-symbol: First::Second::Third::Fourth::NestedClassAlias

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
        #      ^ show-symbol: Outer::Middle::Inner.foo
        # This is kind of ugly. It's possible that we want to return @foo here.
        @foo = T.let(@foo, T.nilable(String))
        #             ^ show-symbol: T.class_of(Outer::Middle::Inner)#@foo
      end

      foo
      # ^ show-symbol: Outer::Middle::Inner.foo

      puts(:foo)
      #    ^ show-symbol: null

      sig {params(x: Integer).void}
      def bar(x)
        # ^ show-symbol: Outer::Middle::Inner#bar
        Kernel.puts(x)
        #           ^ show-symbol: null
      end
    end
  end
end

Hello # error: Unable to resolve constant `Hello`
# ^ show-symbol: null
Hello:: # error: Unable to resolve constant `Hello`
#    ^^ error: expected constant name following "::"
#     ^ show-symbol: null
module A
  Goodbye =
  #        ^ show-symbol: null
end # error: unexpected token "end"
