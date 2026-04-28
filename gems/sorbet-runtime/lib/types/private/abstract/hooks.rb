# frozen_string_literal: true
# typed: true

module T::Private::Abstract::Hooks
  module Helpers
    def abstract!
      @__is_abstract = true
      class << self
        alias_method :__sorbet_orig_new, :new
      end

      extend(Hooks)
    end
  end

  def inherited(other)
    super
    class << other
      alias_method :new, :__sorbet_orig_new
    end
  end

  def new(...)
    result = super
    if @__is_abstract
      raise "#{self} is declared as abstract; it cannot be instantiated"
    end

    result
  end

  def abstract!
    if defined?(super)
      super
    end
    self.define_singleton_method(:new, T::Private::Abstract::Hooks.instance_method(:new))
  end
end
