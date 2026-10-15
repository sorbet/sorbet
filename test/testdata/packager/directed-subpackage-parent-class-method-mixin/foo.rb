# typed: true

module MyPackage
  module M
    extend T::Helpers

    module ClassMethods
      def class_method_from_mixin; end
    end

    mixes_in_class_methods(ClassMethods)
  end

  class Foo
    include M
  end

  Foo.class_method_from_mixin
end
