# frozen_string_literal: true
# typed: true

module T::Private
  module MixesInClassMethods
    def included(other)
      mod = Abstract::Data.get(self, :class_methods_mixin)
      other.extend(mod)
      super
    end
  end

  module Mixins
    def self.declare_mixes_in_class_methods(mixin, class_methods)
      if mixin.is_a?(Class)
        raise "Classes cannot be used as mixins, and so mixes_in_class_methods cannot be used on a Class."
      end

      if Abstract::Data.key?(mixin, :class_methods_mixin)
        raise "mixes_in_class_methods can only be used once per module."
      end

      mixin.singleton_class.include(MixesInClassMethods)
      Abstract::Data.set(mixin, :class_methods_mixin, class_methods)
    end
  end
end
