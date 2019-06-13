# frozen_string_literal: true
# typed: true

module T::Private::Final
  def self.declare(klass)
    if !klass.is_a?(Class)
      raise "#{klass.name} is not a class (it is a #{klass.class}), but was declared final"
    end
    if klass.instance_variable_get(:@sorbet_is_final) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
      raise "#{klass.name} was already declared as final"
    end
    klass.define_singleton_method(:inherited) do |sub|
      raise "#{self.name} was declared final and cannot be subclassed"
    end
    klass.instance_variable_set(:@sorbet_is_final, true) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end
end
