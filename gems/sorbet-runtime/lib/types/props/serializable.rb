# frozen_string_literal: true
# typed: false

module T::Props::Serializable
  include T::Props::Plugin
  # Required because we have special handling for `optional: false`
  include T::Props::Optional
  # Required because we have special handling for extra_props
  include T::Props::PrettyPrintable

  # Serializes this object to a hash, suitable for conversion to
  # JSON/BSON.
  #
  # @param strict [Boolean] (true) If false, do not raise an
  #   exception if this object has mandatory props with missing
  #   values.
  # @return [Hash] A serialization of this object.
  def serialize(strict=true)
    decorator = self.class.decorator
    h = {}

    decorator.props.keys.each do |prop|
      rules = decorator.prop_rules(prop)
      hkey = rules[:serialized_form]

      val = decorator.get(self, prop)

      if strict && val.nil? && T::Props::Utils.need_nil_write_check?(rules)
        raise T::Props::InvalidValueError.new("#{self.class.name}.#{prop} not set for non-optional prop")
      end

      # Don't serialize values that are nil to save space (both the
      # nil value itself and the field name in the serialized BSON
      # document)
      next if decorator.prop_dont_store?(prop) || val.nil?

      if rules[:type_is_serializable]
        val = val.serialize(strict)
      elsif rules[:type_is_array_of_serializable]
        val = val.map {|el| el.serialize(strict)}
      elsif rules[:type_is_hash_of_serializable]
        val = val.transform_values {|v| v.serialize(strict)}
      elsif rules[:type_is_custom_type]
        val = rules[:type].serialize(val)

        unless T::Props::CustomType.valid_serialization?(val, rules[:type])
          msg = "#{rules[:type]} did not serialize to a valid scalar type. It became a: #{val.class}"
          if val.is_a?(Hash)
            msg += "\nIf you want to store a structured Hash, consider using a T::Struct as your type."
          end
          raise T::Props::InvalidValueError.new(msg)
        end
      end

      h[hkey] = T::Props::Utils.deep_clone_object(val)
    end

    extra_props = decorator.extra_props(self)
    h.merge!(extra_props) if extra_props

    h
  end

  # Populates the property values on this object with the values
  # from a hash. In general, prefer to use {.from_hash} to construct
  # a new instance, instead of loading into an existing instance.
  #
  # @param hash [Hash<String, Object>] The hash to take property
  #  values from.
  # @param strict [Boolean] (false) If true, raise an exception if
  #  the hash contains keys that do not correspond to any known
  #  props on this instance.
  # @return [void]
  def deserialize(hash, strict=false)
    decorator = self.class.decorator

    matching_props = 0

    decorator.props.each do |p, rules|
      hkey = rules[:serialized_form]
      val = hash[hkey]
      if val.nil?
        if rules[:optional] == false || rules[:optional] == :migrate
          val = decorator.get_default(rules, self.class)
          if val.nil?
            msg = "Tried to deserialize a required prop from a nil value. It's "\
              "possible that a nil value exists in the database, so you should "\
              "provide a `default: or factory:` for this prop (see go/optional "\
              "for more details). If this is already the case, you probably "\
              "omitted a required prop from the `fields:` option when doing a "\
              "partial load."
            raise msg
          end
        elsif T::Props::Utils.need_nil_read_check?(rules)
          self.required_prop_missing_from_deserialize(p)
        end

        matching_props += 1 if hash.key?(hkey)
      else
        if (subtype = rules[:serializable_subtype])
          val =
            if rules[:type_is_array_of_serializable]
              val.map {|el| subtype.from_hash(el)}
            elsif rules[:type_is_hash_of_serializable]
              val.transform_values {|v| subtype.from_hash(v)}
            else
              subtype.from_hash(val)
            end
        elsif (needs_clone = rules[:type_needs_clone])
          val =
            if needs_clone == :shallow
              val.dup
            else
              T::Props::Utils.deep_clone_object(val)
            end
        elsif rules[:type_is_custom_type]
          val = rules[:type].deserialize(val)
        end

        matching_props += 1
      end

      self.instance_variable_set(rules[:accessor_key], val) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
    end

    # We compute extra_props this way specifically for performance
    if matching_props < hash.size
      pbsf = decorator.prop_by_serialized_forms
      h = hash.reject {|k, _| pbsf.key?(k)}

      if strict
        raise "Unknown properties for #{self.class.name}: #{h.keys.inspect}"
      else
        @_extra_props = h
      end
    end
  end

  # @return [Boolean] Was this property missing during deserialize?
  def required_prop_missing_from_deserialize?(prop)
    return false if @_required_props_missing_from_deserialize.nil?
    @_required_props_missing_from_deserialize.include?(prop)
  end

  # @return Marks this property as missing during deserialize
  def required_prop_missing_from_deserialize(prop)
    @_required_props_missing_from_deserialize ||= Set[]
    @_required_props_missing_from_deserialize << prop
    nil
  end
end


##############################################

# NB: This must stay in the same file where T::Props::Serializable is defined due to
# T::Props::Decorator#apply_plugin; see https://git.corp.stripe.com/stripe-internal/pay-server/blob/fc7f15593b49875f2d0499ffecfd19798bac05b3/chalk/odm/lib/chalk-odm/document_decorator.rb#L716-L717
module T::Props::Serializable::DecoratorMethods
  def valid_props
    super + [
      :dont_store,
      :name,
      :notify_on_nil_write,
      :raise_on_nil_write,
    ]
  end

  def required_props
    @class.props.select {|_, v| v[:optional] == false}.keys
  end

  def prop_dont_store?(prop); prop_rules(prop)[:dont_store]; end
  def prop_by_serialized_forms; @class.prop_by_serialized_forms; end

  def from_hash(hash, strict=false)
    raise ArgumentError.new("#{hash.inspect} provided to from_hash") if !(hash && hash.is_a?(Hash))

    i = @class.allocate
    i.deserialize(hash, strict)

    i
  end

  def prop_serialized_form(prop)
    prop_rules(prop)[:serialized_form]
  end

  def serialized_form_prop(serialized_form)
    prop_by_serialized_forms[serialized_form.to_s] || raise("No such serialized form: #{serialized_form.inspect}")
  end

  def add_prop_definition(prop, rules)
    rules[:serialized_form] = rules.fetch(:name, prop.to_s)
    res = super
    prop_by_serialized_forms[rules[:serialized_form]] = prop
    res
  end

  # This is an implementation detail for T.nilable type.  We convert all T.nilable type to optional: true here.
  # This should be removed when all the code can handle T.nilable directly.
  def prop_defined(name, cls, rules={})
    nilable_type_info = T::Utils::Nilable.get_type_info(cls)
    if nilable_type_info.is_union_type
      if nilable_type_info.non_nilable_type
        # We change this inline so that the caller would get this update.
        rules[:optional] = true
        rules[:optional_setter] = :t_nilable # this should be removed from the super call.
        super(name, nilable_type_info.non_nilable_type, rules)
      else
        # TODO(wei): we should check this as error as we only allow T.nilable union, but can not do it for now.
        # There are some strange errors from autogen that is hitting this.
        super
      end
    else
      super
    end
  end

  def prop_validate_definition!(name, cls, rules, type)
    result = super

    if (rules_name = rules[:name])
      unless rules_name.is_a?(String)
        raise ArgumentError.new("Invalid name in prop #{@class.name}.#{name}: #{rules_name.inspect}")
      end

      validate_prop_name(rules_name)
    end

    if !rules[:raise_on_nil_write].nil? && rules[:raise_on_nil_write] != true
        raise ArgumentError.new("The value of `raise_on_nil_write` if specified must be `true` (given: #{rules[:raise_on_nil_write]}).")
    end

    if rules[:raise_on_nil_write] && rules[:notify_on_nil_write]
      raise ArgumentError.new("You can only specify one of `raise_on_nil_write` and `notify_on_nil_write`")
    end

    result
  end

  def get_id(instance)
    prop = prop_by_serialized_forms['_id']
    if prop
      get(instance, prop)
    else
      nil
    end
  end

  EMPTY_EXTRA_PROPS = {}.freeze
  private_constant :EMPTY_EXTRA_PROPS

  def extra_props(instance)
    get(instance, '_extra_props') || EMPTY_EXTRA_PROPS
  end

  # @override T::Props::PrettyPrintable
  private def inspect_instance_components(instance, multiline:, indent:)
    if (extra_props = extra_props(instance)) && !extra_props.empty?
      pretty_kvs = extra_props.map {|k, v| [k.to_sym, v.inspect]}
      extra = join_props_with_pretty_values(pretty_kvs, multiline: false)
      super + ["@_extra_props=<#{extra}>"]
    else
      super
    end
  end
end


##############################################


# NB: This must stay in the same file where T::Props::Serializable is defined due to
# T::Props::Decorator#apply_plugin; see https://git.corp.stripe.com/stripe-internal/pay-server/blob/fc7f15593b49875f2d0499ffecfd19798bac05b3/chalk/odm/lib/chalk-odm/document_decorator.rb#L716-L717
module T::Props::Serializable::ClassMethods
  def prop_by_serialized_forms; @prop_by_serialized_forms ||= {}; end

  # @!method self.from_hash(hash, strict)
  #
  # Allocate a new instance and call {#deserialize} to load a new
  # object from a hash.
  # @return [Serializable]
  def from_hash(hash, strict=false)
    self.decorator.from_hash(hash, strict)
  end

  # Equivalent to {.from_hash} with `strict` set to true.
  # @return [Serializable]
  def from_hash!(hash)
    self.decorator.from_hash(hash, true)
  end
end
