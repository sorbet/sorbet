# frozen_string_literal: true
# typed: true

module T::Private::Final
  def self.declare(klass)
    if !klass.is_a?(Class)
      raise "#{klass.name} is not a class (it is a #{klass.class}), but was declared final"
    end
    if self.is_final?(klass)
      raise "#{klass.name} was already declared as final"
    end
    if T::AbstractUtils.abstract_module?(klass)
      raise "#{klass.name} was already declared as abstract and cannot also be declared as final"
    end
    klass.define_singleton_method(:inherited) do |sub|
      raise "#{self.name} was declared final and cannot be subclassed"
    end
    klass.instance_variable_set(:@sorbet_is_final, true) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end

  def self.is_final?(klass)
    klass.instance_variable_get(:@sorbet_is_final) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end
end
