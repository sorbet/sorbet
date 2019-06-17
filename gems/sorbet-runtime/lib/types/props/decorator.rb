# frozen_string_literal: true
# typed: false

# NB: This is not actually a decorator. It's just named that way for consistency
# with DocumentDecorator and ModelDecorator (which both seem to have been written
# with an incorrect understanding of the decorator pattern). These "decorators"
# should really just be static methods on private modules (we'd also want/need to
# replace decorator overrides in plugins with class methods that expose the necessary
# functionality).
class T::Props::Decorator
  extend T::Sig

  Rules = T.type_alias(T::Hash[Symbol, T.untyped])
  DecoratedClass = T.type_alias(T.untyped) # T.class_of(T::Props), but that produces circular reference errors in some circumstances
  DecoratedInstance = T.type_alias(T.untyped) # Would be T::Props, but that produces circular reference errors in some circumstances
  PropType = T.type_alias(T.any(T::Types::Base, T::Props::CustomType))
  PropTypeOrClass = T.type_alias(T.any(PropType, Module))

  class NoRulesError < StandardError; end

  sig {params(klass: DecoratedClass).void}
  def initialize(klass)
    @class = klass
    klass.plugins.each do |mod|
      Private.apply_decorator_methods(mod, self)
    end
  end

  # prop stuff
  sig {returns(T::Hash[Symbol, Rules])}
  def props
    @props ||= {}.freeze
  end

  # Try to avoid using this; post-definition mutation of prop rules is
  # surprising and hard to reason about.
  sig {params(prop: Symbol, key: Symbol, value: T.untyped).void}
  def mutate_prop_backdoor!(prop, key, value)
    @props = props.merge(
      prop => props.fetch(prop).merge(key => value).freeze
    ).freeze
  end

  sig {returns(T::Array[Symbol])}
  def all_props; props.keys; end

  sig {params(prop: T.any(Symbol, String)).returns(Rules)}
  def prop_rules(prop); props[prop.to_sym] || raise("No such prop: #{prop.inspect}"); end

  sig {params(prop: Symbol, rules: Rules).void}
  def add_prop_definition(prop, rules)
    prop = prop.to_sym
    override = rules.delete(:override)

    if props.include?(prop) && !override
      raise ArgumentError.new("Attempted to redefine prop #{prop.inspect} that's already defined without specifying :override => true: #{prop_rules(prop)}")
    elsif !props.include?(prop) && override
      raise ArgumentError.new("Attempted to override a prop #{prop.inspect} that doesn't already exist")
    end

    @props = @props.merge(prop => rules.freeze).freeze
  end

  sig {returns(T::Array[Symbol])}
  def valid_props
    %i{
      enum
      foreign
      foreign_hint_only
      ifunset
      immutable
      override
      redaction
      sensitivity
      without_accessors
      clobber_existing_method!
      extra
      optional
      _tnilable
    }
  end

  sig {returns(DecoratedClass)}
  def decorated_class; @class; end

  # Accessors

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  sig do
    params(
      instance: DecoratedInstance,
      prop: T.any(String, Symbol),
      rules: T.nilable(Rules)
    )
    .returns(T.untyped)
  end
  def get(instance, prop, rules=props[prop.to_sym])
    # For backwards compatibility, fall back to reconstructing the accessor key
    # (though it would probably make more sense to raise in that case).
    instance.instance_variable_get(rules ? rules[:accessor_key] : '@' + prop.to_s) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  sig do
    params(
      instance: DecoratedInstance,
      prop: Symbol,
      value: T.untyped,
      rules: T.nilable(Rules)
    )
    .void
  end
  def set(instance, prop, value, rules=props[prop.to_sym])
    # For backwards compatibility, fall back to reconstructing the accessor key
    # (though it would probably make more sense to raise in that case).
    instance.instance_variable_set(rules ? rules[:accessor_key] : '@' + prop.to_s, value) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end

  # Use this to validate that a value will validate for a given prop. Useful for knowing whether a value can be set on a model without setting it.
  sig {params(prop: Symbol, val: T.untyped).void}
  def validate_prop_value(prop, val)
    # This implements a 'public api' on document so that we don't allow callers to pass in rules
    # Rules seem like an implementation detail so it seems good to now allow people to specify them manually.
    check_prop_type(prop, val)
  end

  # Passing in rules here is purely a performance optimization.
  sig {params(prop: Symbol, val: T.untyped, rules: Rules).void}
  private def check_prop_type(prop, val, rules=prop_rules(prop))
    type_object = rules.fetch(:type_object)
    type = rules.fetch(:type)

    # TODO: ideally we'd add `&& rules[:optional] != :existing` to this check
    # (it makes sense to treat those props required in this context), but we'd need
    # to be sure that doesn't break any existing code first.
    if val.nil?
      if !T::Props::Utils.need_nil_write_check?(rules) || (rules.key?(:default) && rules[:default].nil?)
        return
      end

      if rules[:raise_on_nil_write]
        raise T::Props::InvalidValueError.new("Can't set #{@class.name}.#{prop} to #{val.inspect} " \
        "(instance of #{val.class}) - need a #{type}")
      end
    end

    # T::Props::CustomType is not a real object based class so that we can not run real type check call.
    # T::Props::CustomType.valid?() is only a helper function call.
    valid =
      if type.is_a?(T::Props::CustomType) && T::Utils::Props.optional_prop?(rules)
        type.valid?(val)
      else
        type_object.valid?(val)
      end
    if !valid
      raise T::Props::InvalidValueError.new("Can't set #{@class.name}.#{prop} to #{val.inspect} " \
        "(instance of #{val.class}) - need a #{type_object}")
    end
  end

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  # Unlike the other methods that take rules, this one calls prop_rules for
  # the default, which raises if the prop doesn't exist (this maintains
  # preexisting behavior).
  sig do
    params(
      instance: DecoratedInstance,
      prop: Symbol,
      val: T.untyped,
      rules: T.nilable(Rules)
    )
    .void
  end
  def prop_set(instance, prop, val, rules=prop_rules(prop))
    check_prop_type(prop, val, T.must(rules))
    set(instance, prop, val, rules)
  end

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  sig do
    params(
      instance: DecoratedInstance,
      prop: T.any(String, Symbol),
      rules: T.nilable(Rules)
    )
    .returns(T.untyped)
  end
  def prop_get(instance, prop, rules=props[prop.to_sym])
    val = get(instance, prop, rules)

    # NB: Do NOT change this to check `val.nil?` instead. BSON::ByteBuffer overrides `==` such
    # that `== nil` can return true while `.nil?` returns false. Tests will break in mysterious
    # ways. A special thanks to Ruby for enabling this type of bug.
    #
    # One side effect here is that _if_ a class (like BSON::ByteBuffer) defines ==
    # in such a way that instances which are not `nil`, ie are not NilClass, nevertheless
    # are `== nil`, then we will transparently convert such instances to `nil` on read.
    # Yes, our code relies on this behavior (as of writing). :thisisfine:
    if val != nil # rubocop:disable Style/NonNilCheck
      val
    else
      raise NoRulesError.new if !rules
      d = rules[:ifunset]
      if d
        T::Props::Utils.deep_clone_object(d)
      else
        nil
      end
    end
  end

  sig do
    params(
      instance: DecoratedInstance,
      prop: Symbol,
      foreign_class: Module,
      rules: Rules,
      opts: Hash
    )
    .returns(T.untyped)
  end
  def foreign_prop_get(instance, prop, foreign_class, rules=props[prop.to_sym], opts={})
    return if !(value = prop_get(instance, prop, rules))
    foreign_class.load(value, {}, opts)
  end

  sig do
    params(
      name: Symbol,
      cls: Module,
      rules: Rules,
      type: PropTypeOrClass
    )
    .void
  end
  def prop_validate_definition!(name, cls, rules, type)
    validate_prop_name(name)

    if rules.key?(:pii)
      raise ArgumentError.new("The 'pii:' option for props has been renamed " \
        "to 'sensitivity:' (in prop #{@class.name}.#{name})")
    end

    if !(rules.keys - valid_props).empty?
      raise ArgumentError.new("At least one invalid prop arg supplied in #{self}: #{rules.keys.inspect}")
    end

    if (array = rules[:array])
      unless array.is_a?(Module)
        raise ArgumentError.new("Bad class as subtype in prop #{@class.name}.#{name}: #{array.inspect}")
      end
    end

    if !(rules[:clobber_existing_method!]) && !(rules[:without_accessors])
      # TODO: we should really be checking all the methods on `cls`, not just Object
      if Object.instance_methods.include?(name.to_sym)
        raise ArgumentError.new(
          "#{name} can't be used as a prop in #{@class} because a method with " \
          "that name already exists (defined by #{@class.instance_method(name).owner} " \
          "at #{@class.instance_method(name).source_location || '<unknown>'}). " \
          "(If using this name is unavoidable, try `without_accessors: true`.)"
        )
      end
    end

    extra = rules[:extra]
    if !extra.nil? && !extra.is_a?(Hash)
      raise ArgumentError.new("Extra metadata must be a Hash in prop #{@class.name}.#{name}")
    end

    nil
  end

  private def validate_prop_name(name)
    if name !~ /\A[A-Za-z_][A-Za-z0-9_-]*\z/
      raise ArgumentError.new("Invalid prop name in #{@class.name}: #{name}")
    end
  end

  # Check if this cls represents a T.nilable(type)
  sig {params(type: PropTypeOrClass).returns(T::Boolean)}
  private def is_nilable?(type)
    return false if !type.is_a?(T::Types::Union)
    type.types.any? {|t| t == T::Utils.coerce(NilClass)}
  end

  # This converts the type from a T::Type to a regular old ruby class.
  sig {params(type: T::Types::Base).returns(Module)}
  private def convert_type_to_class(type)
    case type
    when T::Types::TypedArray, T::Types::FixedArray
      Array
    when T::Types::TypedHash, T::Types::FixedHash
      Hash
    when T::Types::TypedSet
      Set
    when T::Types::Union
      # The below unwraps our T.nilable types for T::Props if we can.
      # This lets us do things like specify: const T.nilable(String), foreign: Opus::DB::Model::Merchant
      non_nil_type = T::Utils.unwrap_nilable(type)
      if non_nil_type
        convert_type_to_class(non_nil_type)
      else
        Object
      end
    when T::Types::Simple
      type.raw_type
    else
      # This isn't allowed unless whitelisted_for_underspecification is
      # true, due to the check in prop_validate_definition
      Object
    end
  end

  sig do
    params(
      name: T.any(Symbol, String),
      cls: PropTypeOrClass,
      rules: Rules,
    )
    .void
  end
  def prop_defined(name, cls, rules={})
    # TODO(jerry): Create similar soft assertions against false
    if rules[:optional] == true
      T::Configuration.hard_assert_handler(
        'Use of `optional: true` is deprecated, please use `T.nilable(...)` instead.',
        storytime: {
          name: name,
          cls_or_args: cls.to_s,
          args: rules,
          klass: decorated_class.name,
        },
      )
    elsif rules[:optional] == false
      T::Configuration.hard_assert_handler(
        'Use of `optional: :false` is deprecated as it\'s the default value.',
        storytime: {
          name: name,
          cls_or_args: cls.to_s,
          args: rules,
          klass: decorated_class.name,
        },
      )
    elsif rules[:optional] == :on_load
      T::Configuration.hard_assert_handler(
        'Use of `optional: :on_load` is deprecated. You probably want `T.nilable(...)` with :raise_on_nil_write instead.',
        storytime: {
          name: name,
          cls_or_args: cls.to_s,
          args: rules,
          klass: decorated_class.name,
        },
      )
    elsif rules[:optional] == :existing
      T::Configuration.hard_assert_handler(
        'Use of `optional: :existing` is not allowed: you should use use T.nilable (http://go/optional)',
        storytime: {
          name: name,
          cls_or_args: cls.to_s,
          args: rules,
          klass: decorated_class.name,
        },
      )
    end

    if T::Utils::Nilable.is_union_with_nilclass(cls)
      # :_tnilable is introduced internally for performance purpose so that clients do not need to call
      # T::Utils::Nilable.is_tnilable(cls) again.
      # It is strictly internal: clients should always use T::Utils::Props.required_prop?() or
      # T::Utils::Props.optional_prop?() for checking whether a field is required or optional.
      rules[:_tnilable] = true
    end

    name = name.to_sym
    type = cls
    if !cls.is_a?(Module)
      cls = convert_type_to_class(cls)
    end
    type_object = type
    if !(type_object.singleton_class < T::Props::CustomType)
      type_object = smart_coerce(type_object, array: rules[:array], enum: rules[:enum])
    end

    prop_validate_definition!(name, cls, rules, type_object)

    # Retrive the possible underlying object with T.nilable.
    underlying_type_object = T::Utils::Nilable.get_underlying_type_object(type_object)
    type = T::Utils::Nilable.get_underlying_type(type)

    array_subdoc_type = array_subdoc_type(underlying_type_object)
    hash_value_subdoc_type = hash_value_subdoc_type(underlying_type_object)
    hash_key_custom_type = hash_key_custom_type(underlying_type_object)

    sensitivity_and_pii = {sensitivity: rules[:sensitivity]}
    if defined?(Opus) && defined?(Opus::Sensitivity) && defined?(Opus::Sensitivity::Utils)
      sensitivity_and_pii = Opus::Sensitivity::Utils.normalize_sensitivity_and_pii_annotation(sensitivity_and_pii)
    end
    # We check for Class so this is only applied on concrete
    # documents/models; We allow mixins containing props to not
    # specify their PII nature, as long as every class into which they
    # are ultimately included does.
    #
    if defined?(Opus) && defined?(Opus::Sensitivity) && defined?(Opus::Sensitivity::PIIable)
      if sensitivity_and_pii[:pii] && @class.is_a?(Class) && !@class.contains_pii?
        raise ArgumentError.new(
          'Cannot include a pii prop in a class that declares `contains_no_pii`'
        )
      end
    end

    needs_clone =
      if cls <= Array || cls <= Hash || cls <= Set
        shallow_clone_ok(underlying_type_object) ? :shallow : true
      else
        false
      end

    rules = rules.merge(
      # TODO: The type of this element is confusing. We should refactor so that
      # it can be always `type_object` (a PropType) or always `cls` (a Module)
      type: type,
      # These are precomputed for performance
      # TODO: A lot of these are only needed by T::Props::Serializable or T::Struct
      # and can/should be moved accordingly.
      type_is_custom_type: cls.singleton_class < T::Props::CustomType,
      type_is_serializable: cls < T::Props::Serializable,
      type_is_array_of_serializable: !array_subdoc_type.nil?,
      type_is_hash_of_serializable_values: !hash_value_subdoc_type.nil?,
      type_is_hash_of_custom_type_keys: !hash_key_custom_type.nil?,
      type_object: type_object,
      type_needs_clone: needs_clone,
      accessor_key: "@#{name}".to_sym,
      sensitivity: sensitivity_and_pii[:sensitivity],
      pii: sensitivity_and_pii[:pii],
      # extra arbitrary metadata attached by the code defining this property
      extra: rules[:extra]&.freeze,
    )

    validate_not_missing_sensitivity(name, rules)

    # for backcompat
    if type.is_a?(T::Types::TypedArray) && type.type.is_a?(T::Types::Simple)
      rules[:array] = type.type.raw_type
    elsif array_subdoc_type
      rules[:array] = array_subdoc_type
    end

    if rules[:type_is_serializable]
      rules[:serializable_subtype] = cls
    elsif array_subdoc_type
      rules[:serializable_subtype] = array_subdoc_type
    elsif hash_value_subdoc_type && hash_key_custom_type
      rules[:serializable_subtype] = {
        keys: hash_key_custom_type,
        values: hash_value_subdoc_type,
      }
    elsif hash_value_subdoc_type
      rules[:serializable_subtype] = hash_value_subdoc_type
    elsif hash_key_custom_type
      rules[:serializable_subtype] = hash_key_custom_type
    end

    add_prop_definition(name, rules)
    # NB: using `without_accessors` doesn't make much sense unless you also define some other way to
    # get at the property (e.g., Chalk::ODM::Document exposes `get` and `set`).
    define_getter_and_setter(name, rules) unless rules[:without_accessors]

    if rules[:foreign] && rules[:foreign_hint_only]
      raise ArgumentError.new(":foreign and :foreign_hint_only are mutually exclusive.")
    end

    handle_foreign_option(name, cls, rules, rules[:foreign]) if rules[:foreign]
    handle_foreign_hint_only_option(cls, rules[:foreign_hint_only]) if rules[:foreign_hint_only]
    handle_redaction_option(name, rules[:redaction]) if rules[:redaction]
  end

  sig {params(name: Symbol, rules: Rules).void}
  private def define_getter_and_setter(name, rules)
    if rules[:immutable]
      @class.send(:define_method, "#{name}=") do |_x|
        raise T::Props::ImmutableProp.new("#{self.class}##{name} cannot be modified after creation.")
      end
    else
      @class.send(:define_method, "#{name}=") do |x|
        self.class.decorator.prop_set(self, name, x, rules)
      end
    end

    @class.send(:define_method, name) do
      self.class.decorator.prop_get(self, name, rules)
    end
  end

  # returns the subdoc of the array type, or nil if it's not a Document type
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
  end
  private def array_subdoc_type(type)
    if type.is_a?(T::Types::TypedArray)
      el_type = T::Utils.unwrap_nilable(type.type) || type.type

      if el_type.is_a?(T::Types::Simple) &&
          (el_type.raw_type < T::Props::Serializable || el_type.raw_type.is_a?(T::Props::CustomType))
        return el_type.raw_type
      end
    end

    nil
  end

  # returns the subdoc of the hash value type, or nil if it's not a Document type
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
  end
  private def hash_value_subdoc_type(type)
    if type.is_a?(T::Types::TypedHash)
      values_type = T::Utils.unwrap_nilable(type.values) || type.values

      if values_type.is_a?(T::Types::Simple) &&
          (values_type.raw_type < T::Props::Serializable || values_type.raw_type.is_a?(T::Props::CustomType))
        return values_type.raw_type
      end
    end

    nil
  end

  # returns the type of the hash key, or nil. Any CustomType could be a key, but we only expect Opus::Enum right now.
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
  end
  private def hash_key_custom_type(type)
    if type.is_a?(T::Types::TypedHash)
      keys_type = T::Utils.unwrap_nilable(type.keys) || type.keys

      if keys_type.is_a?(T::Types::Simple) && keys_type.raw_type.is_a?(T::Props::CustomType)
        return keys_type.raw_type
      end
    end

    nil
  end

  # From T::Props::Utils.deep_clone_object, plus String
  TYPES_NOT_NEEDING_CLONE = [TrueClass, FalseClass, NilClass, Symbol, String, Numeric]
  if defined?(Opus) && defined?(Opus::Enum)
    TYPES_NOT_NEEDING_CLONE << Opus::Enum
  end

  sig {params(type: PropType).returns(T::Boolean)}
  private def shallow_clone_ok(type)
    inner_type =
      if type.is_a?(T::Types::TypedArray)
        type.type
      elsif type.is_a?(T::Types::TypedSet)
        type.type
      elsif type.is_a?(T::Types::TypedHash)
        type.values
      end

    inner_type.is_a?(T::Types::Simple) && TYPES_NOT_NEEDING_CLONE.any? do |cls|
      inner_type.raw_type <= cls
    end
  end

  sig do
    params(type: PropTypeOrClass, array: T.untyped, enum: T.untyped)
    .returns(T::Types::Base)
  end
  private def smart_coerce(type, array:, enum:)
    # Backwards compatibility for pre-T::Types style
    if !array.nil? && !enum.nil?
      raise ArgumentError.new("Cannot specify both :array and :enum options")
    elsif !array.nil?
      if type == Set
        T::Set[array]
      else
        T::Array[array]
      end
    elsif !enum.nil?
      if T::Utils.unwrap_nilable(type)
        T.nilable(T.enum(enum))
      else
        T.enum(enum)
      end
    else
      T::Utils.coerce(type)
    end
  end

  sig {params(prop_name: Symbol, rules: Hash).void}
  private def validate_not_missing_sensitivity(prop_name, rules)
    if rules[:sensitivity].nil?
      if rules[:redaction]
        T::Configuration.hard_assert_handler(
          "#{@class}##{prop_name} has a 'redaction:' annotation but no " \
          "'sensitivity:' annotation. This is probably wrong, because if a " \
          "prop needs redaction then it is probably sensitive. Add a " \
          "sensitivity annotation like 'sensitivity: Opus::Sensitivity::PII." \
          "whatever', or explicitly override this check with 'sensitivity: []'."
        )
      end
      # TODO(PRIVACYENG-982) Ideally we'd also check for 'password' and possibly
      # other terms, but this interacts badly with ProtoDefinedDocument because
      # the proto syntax currently can't declare "sensitivity: []"
      if prop_name =~ /\bsecret\b/
        T::Configuration.hard_assert_handler(
          "#{@class}##{prop_name} has the word 'secret' in its name, but no " \
          "'sensitivity:' annotation. This is probably wrong, because if a " \
          "prop is named 'secret' then it is probably sensitive. Add a " \
          "sensitivity annotation like 'sensitivity: Opus::Sensitivity::NonPII." \
          "security_token', or explicitly override this check with " \
          "'sensitivity: []'."
        )
      end
    end
  end

  # Create "#{prop_name}_redacted" method
  sig do
    params(
      prop_name: Symbol,
      redaction: Chalk::Tools::RedactionUtils::RedactionDirectiveSpec,
    )
    .void
  end
  private def handle_redaction_option(prop_name, redaction)
    redacted_method = "#{prop_name}_redacted"

    @class.send(:define_method, redacted_method) do
      value = self.public_send(prop_name)
      Chalk::Tools::RedactionUtils.redact_with_directive(
        value, redaction)
    end
  end

  sig do
    params(
      option_sym: Symbol,
      foreign: T.untyped,
      valid_type_msg: String,
    )
    .void
  end
  private def validate_foreign_option(option_sym, foreign, valid_type_msg:)
    if foreign.is_a?(Symbol) || foreign.is_a?(String)
      raise ArgumentError.new(
        "Using a symbol/string for `#{option_sym}` is no longer supported. Instead, use a Proc " \
        "that returns the class, e.g., foreign: -> {Foo}"
      )
    end

    if !foreign.is_a?(Proc) && !foreign.is_a?(Array) && !foreign.respond_to?(:load)
      raise ArgumentError.new("The `#{option_sym}` option must be #{valid_type_msg}")
    end
  end

  sig do
    params(
      prop_cls: Module,
      foreign_hint_only: T.untyped,
    )
    .void
  end
  private def handle_foreign_hint_only_option(prop_cls, foreign_hint_only)
    if ![String, Array].include?(prop_cls) && !(prop_cls.is_a?(T::Props::CustomType))
      raise ArgumentError.new(
        "`foreign_hint_only` can only be used with String or Array prop types"
      )
    end

    validate_foreign_option(
      :foreign_hint_only, foreign_hint_only,
      valid_type_msg: "an individual or array of a model class, or a Proc returning such."
    )
  end

  sig do
    params(
      prop_name: T.any(String, Symbol),
      rules: Rules,
      foreign: T.untyped,
    )
    .void
  end
  private def define_foreign_method(prop_name, rules, foreign)
    fk_method = "#{prop_name}_"

    # n.b. there's no clear reason *not* to allow additional options
    # here, but we're baking in `allow_direct_mutation` since we
    # *haven't* allowed additional options in the past and want to
    # default to keeping this interface narrow.
    @class.send(:define_method, fk_method) do |allow_direct_mutation: nil|
      if foreign.is_a?(Proc)
        resolved_foreign = foreign.call
        if !resolved_foreign.respond_to?(:load)
          raise ArgumentError.new(
            "The `foreign` proc for `#{prop_name}` must return a model class. " \
            "Got `#{resolved_foreign.inspect}` instead."
          )
        end
        # `foreign` is part of the closure state, so this will persist to future invocations
        # of the method, optimizing it so this only runs on the first invocation.
        foreign = resolved_foreign
      end
      if allow_direct_mutation.nil?
        opts = {}
      else
        opts = {allow_direct_mutation: allow_direct_mutation}
      end

      self.class.decorator.foreign_prop_get(self, prop_name, foreign, rules, opts)
    end

    force_fk_method = "#{fk_method}!"
    @class.send(:define_method, force_fk_method) do |allow_direct_mutation: nil|
      loaded_foreign = send(fk_method, allow_direct_mutation: allow_direct_mutation)
      if !loaded_foreign
        T::Configuration.hard_assert_handler(
          'Failed to load foreign model',
          storytime: {method: force_fk_method, class: self.class}
        )
      end
      T.must(loaded_foreign)
    end

    @class.send(:define_method, "#{prop_name}_record") do |allow_direct_mutation: nil|
      T::Configuration.soft_assert_handler(
        "Using deprecated 'model.#{prop_name}_record' foreign key syntax. You should replace this with 'model.#{prop_name}_'",
        notify: 'vasi'
      )
      send(fk_method, allow_direct_mutation: allow_direct_mutation)
    end
  end

  sig do
    params(
      prop_name: Symbol,
      prop_cls: Module,
      rules: Rules,
      foreign: T.untyped,
    )
    .void
  end
  private def handle_foreign_option(prop_name, prop_cls, rules, foreign)
    validate_foreign_option(
      :foreign, foreign, valid_type_msg: "a model class or a Proc that returns one"
    )

    if prop_cls != String
      raise ArgumentError.new("`foreign` can only be used with a prop type of String")
    end

    if foreign.is_a?(Array)
      # We don't support arrays with `foreign` because it's hard to both preserve ordering and
      # keep them from being lurky performance hits by issuing a bunch of un-batched DB queries.
      # We could potentially address that by porting over something like AmbiguousIDLoader.
      raise ArgumentError.new(
        "Using an array for `foreign` is no longer supported. Instead, use `foreign_hint_only` " \
        "with an array or a Proc that returns an array, e.g., foreign_hint_only: -> {[Foo, Bar]}"
      )
    end

    define_foreign_method(prop_name, rules, foreign)
  end

  # TODO: rename this to props_inherited
  #
  # This gets called when a module or class that extends T::Props gets included, extended,
  # prepended, or inherited.
  sig {params(child: DecoratedClass).void}
  def model_inherited(child)
    child.extend(T::Props::ClassMethods)
    child.plugins.concat(decorated_class.plugins)

    decorated_class.plugins.each do |mod|
      # NB: apply_class_methods must not be an instance method on the decorator itself,
      # otherwise we'd have to call child.decorator here, which would create the decorator
      # before any `decorator_class` override has a chance to take effect (see the comment below).
      Private.apply_class_methods(mod, child)
    end

    props.each do |name, rules|
      copied_rules = rules.dup
      # NB: Calling `child.decorator` here is a timb bomb that's going to give someone a really bad
      # time. Any class that defines props and also overrides the `decorator_class` method is going
      # to reach this line before its override take effect, turning it into a no-op.
      child.decorator.add_prop_definition(name, copied_rules)
    end
  end

  sig {params(mod: Module).void}
  def plugin(mod)
    decorated_class.plugins << mod
    Private.apply_class_methods(mod, decorated_class)
    Private.apply_decorator_methods(mod, self)
  end

  module Private
    # These need to be non-instance methods so we can use them without prematurely creating the
    # child decorator in `model_inherited` (see comments there for details).
    def self.apply_class_methods(plugin, target)
      if plugin.const_defined?('ClassMethods')
        # FIXME: This will break preloading, selective test execution, etc if `mod::ClassMethods`
        # is ever defined in a separate file from `mod`.
        target.extend(plugin::ClassMethods) # rubocop:disable PrisonGuard/NoDynamicConstAccess
      end
    end

    def self.apply_decorator_methods(plugin, target)
      if plugin.const_defined?('DecoratorMethods')
        # FIXME: This will break preloading, selective test execution, etc if `mod::DecoratorMethods`
        # is ever defined in a separate file from `mod`.
        target.extend(plugin::DecoratorMethods) # rubocop:disable PrisonGuard/NoDynamicConstAccess
      end
    end
  end
end
