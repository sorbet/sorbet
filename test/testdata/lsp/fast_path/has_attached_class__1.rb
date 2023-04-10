# typed: true
extend T::Sig

module Inheritable
  extend T::Helpers

  module ClassMethods
    extend T::Generic
    has_attached_class!
  end
  mixes_in_class_methods(ClassMethods)
end
