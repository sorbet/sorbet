# frozen_string_literal: true
# typed: true

module T::Props::Plugin
  include T::Props
  extend T::Helpers

  module ClassMethods
    def included(child)
      super
      child.plugin(self)
    end
  end
  mixes_in_class_methods(ClassMethods)
end
