# typed: true
require_relative './gems/sorbet-runtime/lib/sorbet-runtime'
require_relative './patch-extend'

module GrandParent
  extend T::Helpers
  module CMs; end
  mixes_in_class_methods(CMs)
end

module Parent
  include T::Props::Plugin
  include GrandParent
end

class Child
  include Parent
end

p Parent.singleton_class.ancestors.include?(GrandParent::CMs)
p Child.singleton_class.ancestors.include?(GrandParent::CMs)
