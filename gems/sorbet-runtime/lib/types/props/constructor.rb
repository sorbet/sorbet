# frozen_string_literal: true
# typed: false

module T::Props::Constructor
  include T::Props::WeakConstructor

  def initialize(hash={})
    decorator = self.class.decorator

    decorator.props.each do |prop, rules|
      # It's important to explicitly compare against `true` here; the value can also be :existing or
      # :on_load (which are truthy) but we don't want to treat those as optional in this context.
      if T::Utils::Props.required_prop?(rules) && !decorator.has_default?(rules) && !hash.key?(prop)
        raise ArgumentError.new("Missing required prop `#{prop}` for class `#{self.class}`")
      end
    end

    super
  end
end
