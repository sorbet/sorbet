# frozen_string_literal: true
# typed: false

module T::Props::Optional
  include T::Props::Plugin
end


##############################################


# NB: This must stay in the same file where T::Props::Optional is defined due to
# T::Props::Decorator#apply_plugin; see https://git.corp.stripe.com/stripe-internal/pay-server/blob/fc7f15593b49875f2d0499ffecfd19798bac05b3/chalk/odm/lib/chalk-odm/document_decorator.rb#L716-L717
module T::Props::Optional::DecoratorMethods
  # TODO: clean this up. This set of options is confusing, and some of them are not universally
  # applicable (e.g., :on_load only applies when using T::Serializable).
  VALID_OPTIONAL_RULES = Set[
    :existing, # deprecated
    :on_load,
    false,
    true,
  ].freeze

  def valid_props
    super + [
      :default,
      :factory,
      :optional,
    ]
  end

  def prop_optional?(prop); prop_rules(prop)[:fully_optional]; end

  def add_prop_definition(prop, rules)
    rules[:fully_optional] = !T::Props::Utils.need_nil_write_check?(rules)
    super
  end

  def prop_validate_definition!(name, cls, rules, type)
    result = super

    if (rules_optional = rules[:optional])
      if !VALID_OPTIONAL_RULES.include?(rules_optional)
        raise ArgumentError.new(":optional must be one of #{VALID_OPTIONAL_RULES.inspect}")
      end
    end

    if rules.key?(:default) && rules.key?(:factory)
      raise ArgumentError.new("Setting both :default and :factory is invalid. See: go/chalk-docs")
    end

    result
  end

  def has_default?(rules)
    rules.include?(:default) || rules.include?(:factory)
  end

  def get_default(rules, instance_class)
    if rules.include?(:default)
      default = rules[:default]
      T::Props::Utils.deep_clone_object(default)
    elsif rules.include?(:factory)
      # Factory should never be nil if the key is specified, but
      # we do this rather than 'elsif rules[:factory]' for
      # consistency with :default.
      factory = rules[:factory]
      instance_class.class_exec(&factory)
    else
      nil
    end
  end
end
