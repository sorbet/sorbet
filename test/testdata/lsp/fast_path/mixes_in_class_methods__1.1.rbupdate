# typed: true

module Opus::TsGen
  module TypeGenerator
    extend T::Helpers
    interface!

    module MyClassMethods
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.void}
      def default; end
    end
    mixes_in_class_methods(MyClassMethods)
  end

  module DocsTypeGenerator
    include TypeGenerator

    extend T::Sig
    sig { override.void }
    def self.default
    end

  end
end
