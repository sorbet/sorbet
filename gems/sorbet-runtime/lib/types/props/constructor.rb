# frozen_string_literal: true
# typed: false

module T::Props::Constructor
  include T::Props::WeakConstructor
end

module T::Props::Constructor::DecoratorMethods
  extend T::Sig

  # Set values for all props that have no defaults. Override what `WeakConstructor`
  # does in order to raise errors on nils instead of ignoring them.
  #
  # @return [Integer] A count of props that we successfully initialized (which
  # we'll use to check for any unrecognized input.)
  #
  # checked(:never) - O(runtime object construction)
  sig {params(instance: T::Props::Constructor, hash: T::Hash[Symbol, T.untyped]).returns(Integer).checked(:never)}
  def construct_props_without_defaults(instance, hash)
    @props_without_defaults&.count do |p, setter_proc|
      begin
        val = hash[p]
        instance.instance_exec(val, &setter_proc)
        val || hash.key?(p)
      rescue TypeError, T::Props::InvalidValueError
        if !hash.key?(p)
          raise ArgumentError.new("Missing required prop `#{p}` for class `#{instance.class.name}`")
        else
          raise
        end
      end
    end || 0
  end
end
