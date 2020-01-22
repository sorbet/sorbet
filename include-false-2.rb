# typed: true
require_relative './gems/sorbet-runtime/lib/sorbet-runtime'
require_relative './patch-extend'

module GrandParent
  extend T::Helpers
  module ClassMethods; end
  mixes_in_class_methods(ClassMethods)
end

module Parent
  include GrandParent
  include T::Props::Plugin
end

class Child
  include Parent
end

p Parent.singleton_class.ancestors.include?(GrandParent::ClassMethods)
p Child.singleton_class.ancestors.include?(GrandParent::ClassMethods)
