# typed: strict
# selective-apply-code-action: refactor.extract

module Foo
  module Bar
    module Qux
      extend T::Sig
      sig {void}
      def self.greeting; end
         # | apply-code-action: [A] Move method to a new module
    end
  end
end

