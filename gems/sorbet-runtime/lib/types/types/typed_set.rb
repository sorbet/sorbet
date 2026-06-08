# frozen_string_literal: true
# typed: true

module T::Types
  class TypedSet < TypedEnumerable
    module Private
      # Cell holding the resolved ::Set module, or nil while Set is
      # autoload-pending / not yet defined (no object can be a Set until
      # then, and the probes must be re-run on each call). Once resolved,
      # per-call valid? skips the Object.autoload?/const_defined?
      # constant-table probes forever. A one-element cell rather than a
      # class ivar so instance methods can read it without a singleton
      # dispatch; racy writes all store the same ::Set and are benign.
      RESOLVED_SET = [nil] # rubocop:disable Style/MutableConstant

      def self.resolve_set
        return nil if Object.autoload?(:Set) # Set is meant to be autoloaded but not yet loaded, this value can't be a Set
        return nil unless Object.const_defined?(:Set) # Set is not loaded yet
        RESOLVED_SET[0] = ::Set
      end
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
      set_mod = Private::RESOLVED_SET[0] || Private.resolve_set
      return false if set_mod.nil?
      return false unless obj.is_a?(set_mod)
      type_ = self.type
      obj.each do |item|
        return false unless type_.recursively_valid?(item)
      end
      true
    end

    # overrides Base
    def valid?(obj)
      set_mod = Private::RESOLVED_SET[0] || Private.resolve_set
      return false if set_mod.nil?
      obj.is_a?(set_mod)
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
        set_mod = TypedSet::Private::RESOLVED_SET[0] || TypedSet::Private.resolve_set
        return false if set_mod.nil?
        obj.is_a?(set_mod)
      end

      # overrides TypedSet
      #
      # Every element trivially satisfies T.untyped, so the inherited O(n)
      # element walk is pure overhead.
      def recursively_valid?(obj)
        set_mod = TypedSet::Private::RESOLVED_SET[0] || TypedSet::Private.resolve_set
        return false if set_mod.nil?
        obj.is_a?(set_mod)
      end
    end
  end
end
