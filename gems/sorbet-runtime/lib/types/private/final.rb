# frozen_string_literal: true
# typed: true

module T::Private::Final
  FinalMarker = "@sorbet_final_marker"

  def self.declare(klass)
    if !klass.is_a?(Class)
      raise "#{klass.name} is not a class, but was declared final"
    end
    if self.is_final?(klass)
      raise "#{klass.name} was already declared as final"
    end
    klass.send(:define_singleton_method, :inherited) do |sub|
      raise "#{self.name} was declared final and cannot be subclassed"
    end
    self.mark_as_final(klass)
  end

  def self.is_final?(klass)
    klass.instance_variable_get(FinalMarker) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end

  def self.mark_as_final(klass)
    klass.instance_variable_set(FinalMarker, true) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end
end
