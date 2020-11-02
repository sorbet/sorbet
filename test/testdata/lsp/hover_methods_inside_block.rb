# typed: true

module Foo
  module Bar
    module Baz
      extend T::Sig
      extend T::Helpers

      sig {returns(T::Set[String])}
      def enabled_methods
        Set.new
      end

      sig {void}
      def foo
        errors = enabled_methods
          .filter_map {|(_, foo)| foo}
          .flat_map {|validation| validation.public_send(validation)} # error-with-dupes: This code is unreachable
                                               # ^ hover: def public_send(arg0, *args); end
        errors
      end
    end
  end
end
