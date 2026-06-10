# frozen_string_literal: true
# typed: true

module T::Types
  class TypedSet < TypedEnumerable
    # Latch holding the resolved ::Set module. nil while Set is
    # autoload-pending or undefined (no object can be a Set until then, and
    # the constant-table probes must re-run on each call); once Set loads,
    # the latch holds ::Set forever and per-call valid? skips the
    # Object.autoload?/const_defined? probes. A class variable so instance
    # methods here and in subclasses (Untyped) read it without a singleton
    # dispatch. Racy writes all store the same ::Set and are benign; no lock
    # (trap-handler safe).
    @@set_module = nil # rubocop:disable Style/ClassVars

    private def resolve_set_module
      return nil if Object.autoload?(:Set) # Set is meant to be autoloaded but not yet loaded, this value can't be a Set
      return nil unless Object.const_defined?(:Set) # Set is not loaded yet
      @@set_module = ::Set # rubocop:disable Style/ClassVars
    end

    def underlying_class
      Set
    end

    # overrides Base
    def name
      "T::Set[#{type.name}]"
    end

    # overrides Base
    def recursively_valid?(obj)
      set_module = @@set_module || resolve_set_module
      return false if set_module.nil?
      obj.is_a?(set_module) && super
    end

    # overrides Base
    def valid?(obj)
      set_module = @@set_module || resolve_set_module
      return false if set_module.nil?
      obj.is_a?(set_module)
    end

    def new(...)
      # Fine for this to blow up, because hopefully if they're trying to make a
      # Set, they don't mind putting (or already have put) a `require 'set'` in
      # their program directly.
      Set.new(...)
    end

    class Untyped < TypedSet
      def initialize
        super(T::Types::Untyped::Private::INSTANCE)
      end

      def valid?(obj)
        set_module = @@set_module || resolve_set_module
        return false if set_module.nil?
        obj.is_a?(set_module)
      end
    end
  end
end
