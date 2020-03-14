# frozen_string_literal: true
# typed: strict

# NB: This is not actually a decorator. It's just named that way for consistency
# with DocumentDecorator and ModelDecorator (which both seem to have been written
# with an incorrect understanding of the decorator pattern). These "decorators"
# should really just be static methods on private modules (we'd also want/need to
# replace decorator overrides in plugins with class methods that expose the necessary
# functionality).
class T::Props::Decorator
  extend T::Sig

  Rules = T.type_alias {T::Hash[Symbol, T.untyped]}
  DecoratedInstance = T.type_alias {Object} # Would be T::Props, but that produces circular reference errors in some circumstances
  PropType = T.type_alias {T.any(T::Types::Base, T::Props::CustomType)}
  PropTypeOrClass = T.type_alias {T.any(PropType, Module)}

  class NoRulesError < StandardError; end

  EMPTY_PROPS = T.let({}.freeze, T::Hash[Symbol, Rules])
  private_constant :EMPTY_PROPS

  sig {params(klass: T.untyped).void.checked(:never)}
  def initialize(klass)
    @class = T.let(klass, T.all(Module, T::Props::ClassMethods))
    @class.plugins.each do |mod|
      T::Props::Plugin::Private.apply_decorator_methods(mod, self)
    end
    @props = T.let(EMPTY_PROPS, T::Hash[Symbol, Rules])
  end

  # checked(:never) - O(prop accesses)
  sig {returns(T::Hash[Symbol, Rules]).checked(:never)}
  attr_reader :props

  sig {returns(T::Array[Symbol])}
  def all_props; props.keys; end

  # checked(:never) - O(prop accesses)
  sig {params(prop: T.any(Symbol, String)).returns(Rules).checked(:never)}
  def prop_rules(prop); props[prop.to_sym] || raise("No such prop: #{prop.inspect}"); end

  # checked(:never) - Rules hash is expensive to check
  sig {params(prop: Symbol, rules: Rules).void.checked(:never)}
  def add_prop_definition(prop, rules)
    override = rules.delete(:override)

    if props.include?(prop) && !override
      raise ArgumentError.new("Attempted to redefine prop #{prop.inspect} that's already defined without specifying :override => true: #{prop_rules(prop)}")
    elsif !props.include?(prop) && override
      raise ArgumentError.new("Attempted to override a prop #{prop.inspect} that doesn't already exist")
    end

    @props = @props.merge(prop => rules.freeze).freeze
  end

  VALID_RULE_KEYS = T.let(%i{
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
    setter_validate
    _tnilable
  }.map {|k| [k, true]}.to_h.freeze, T::Hash[Symbol, T::Boolean])
  private_constant :VALID_RULE_KEYS

  sig {params(key: Symbol).returns(T::Boolean).checked(:never)}
  def valid_rule_key?(key)
    !!VALID_RULE_KEYS[key]
  end

  # checked(:never) - O(prop accesses)
  sig {returns(T.all(Module, T::Props::ClassMethods)).checked(:never)}
  def decorated_class; @class; end

  # Accessors

  # Use this to validate that a value will validate for a given prop. Useful for knowing whether a value can be set on a model without setting it.
  #
  # checked(:never) - potentially O(prop accesses) depending on usage pattern
  sig {params(prop: Symbol, val: T.untyped).void.checked(:never)}
  def validate_prop_value(prop, val)
    # We call `setter_proc` here without binding to an instance, so it'll run
    # `instance_variable_set` if validation passes, but nothing will care.
    # We only care about the validation.
    prop_rules(prop).fetch(:setter_proc).call(val)
  end

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  # Unlike the other methods that take rules, this one calls prop_rules for
  # the default, which raises if the prop doesn't exist (this maintains
  # preexisting behavior).
  #
  # Note this path is NOT used by generated setters on instances,
  # which are defined using `setter_proc` directly.
  #
  # checked(:never) - O(prop accesses)
  sig do
    params(
      instance: DecoratedInstance,
      prop: Symbol,
      val: T.untyped,
      rules: Rules
    )
    .void
    .checked(:never)
  end
  def prop_set(instance, prop, val, rules=prop_rules(prop))
    instance.instance_exec(val, &rules.fetch(:setter_proc))
  end
  alias_method :set, :prop_set

  # For performance, don't use named params here.
  # Passing in rules here is purely a performance optimization.
  #
  # Note this path is NOT used by generated getters on instances,
  # unless `ifunset` is used on the prop, or `prop_get` is overridden.
  #
  # checked(:never) - O(prop accesses)
  sig do
    params(
      instance: DecoratedInstance,
      prop: T.any(String, Symbol),
      rules: Rules
    )
    .returns(T.untyped)
    .checked(:never)
  end
  def prop_get(instance, prop, rules=prop_rules(prop))
    val = instance.instance_variable_get(rules[:accessor_key])
    if !val.nil?
      val
    else
      if (d = rules[:ifunset])
        T::Props::Utils.deep_clone_object(d)
      else
        nil
      end
    end
  end

  sig do
    params(
      instance: DecoratedInstance,
      prop: T.any(String, Symbol),
      rules: Rules
    )
    .returns(T.untyped)
    .checked(:never)
  end
  def prop_get_if_set(instance, prop, rules=prop_rules(prop))
    instance.instance_variable_get(rules[:accessor_key])
  end
  alias_method :get, :prop_get_if_set # Alias for backwards compatibility

  # checked(:never) - O(prop accesses)
  sig do
    params(
      instance: DecoratedInstance,
      prop: Symbol,
      foreign_class: Module,
      rules: Rules,
      opts: T::Hash[Symbol, T.untyped],
    )
    .returns(T.untyped)
    .checked(:never)
  end
  def foreign_prop_get(instance, prop, foreign_class, rules=props[prop.to_sym], opts={})
    return if !(value = prop_get(instance, prop, rules))
    T.unsafe(foreign_class).load(value, {}, opts)
  end

  # TODO: we should really be checking all the methods on `cls`, not just Object
  BANNED_METHOD_NAMES = T.let(Object.instance_methods.to_set.freeze, T::Set[Symbol])

  # checked(:never) - Rules hash is expensive to check
  sig do
    params(
      name: Symbol,
      cls: Module,
      rules: Rules,
      type: PropTypeOrClass
    )
    .void
    .checked(:never)
  end
  def prop_validate_definition!(name, cls, rules, type)
    validate_prop_name(name)

    if rules.key?(:pii)
      raise ArgumentError.new("The 'pii:' option for props has been renamed " \
        "to 'sensitivity:' (in prop #{@class.name}.#{name})")
    end

    if rules.keys.any? {|k| !valid_rule_key?(k)}
      raise ArgumentError.new("At least one invalid prop arg supplied in #{self}: #{rules.keys.inspect}")
    end

    if (array = rules[:array])
      unless array.is_a?(Module)
        raise ArgumentError.new("Bad class as subtype in prop #{@class.name}.#{name}: #{array.inspect}")
      end
    end

    if !(rules[:clobber_existing_method!]) && !(rules[:without_accessors])
      if BANNED_METHOD_NAMES.include?(name.to_sym)
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

  SAFE_NAME = /\A[A-Za-z_][A-Za-z0-9_-]*\z/

  # Used to validate both prop names and serialized forms
  sig {params(name: T.any(Symbol, String)).void}
  private def validate_prop_name(name)
    if !name.match?(SAFE_NAME)
      raise ArgumentError.new("Invalid prop name in #{@class.name}: #{name}")
    end
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

  # checked(:never) - Rules hash is expensive to check
  sig do
    params(
      name: T.any(Symbol, String),
      cls: PropTypeOrClass,
      rules: Rules,
    )
    .void
    .checked(:never)
  end
  def prop_defined(name, cls, rules={})
    cls = T::Utils.resolve_alias(cls)
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
      # It is strictly internal: clients should always use T::Props::Utils.required_prop?() or
      # T::Props::Utils.optional_prop?() for checking whether a field is required or optional.
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
      if sensitivity_and_pii[:pii] && @class.is_a?(Class) && !T.unsafe(@class).contains_pii?
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
      # TODO: The `type_is_*` properties are no longer used internally and should
      # be removed once pay-server no longer depends on them.
      type_is_serializable: cls < T::Props::Serializable,
      type_is_array_of_serializable: !array_subdoc_type.nil?,
      type_is_hash_of_serializable_values: !hash_value_subdoc_type.nil?,
      type_object: type_object,
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

    # TODO: `serializable_subtype` is no longer used internally and should
    # be removed once pay-server no longer depends on it.
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

    rules[:setter_proc] = T::Props::Private::SetterFactory.build_setter_proc(@class, name, rules).freeze

    add_prop_definition(name, rules)

    # NB: using `without_accessors` doesn't make much sense unless you also define some other way to
    # get at the property (e.g., Chalk::ODM::Document exposes `get` and `set`).
    define_getter_and_setter(name, rules) unless rules[:without_accessors]

    if rules[:foreign] && rules[:foreign_hint_only]
      raise ArgumentError.new(":foreign and :foreign_hint_only are mutually exclusive.")
    end

    handle_foreign_option(name, cls, rules, rules[:foreign]) if rules[:foreign]
    handle_foreign_hint_only_option(name, cls, rules[:foreign_hint_only]) if rules[:foreign_hint_only]
    handle_redaction_option(name, rules[:redaction]) if rules[:redaction]
  end

  # checked(:never) - Rules hash is expensive to check
  sig {params(name: Symbol, rules: Rules).void.checked(:never)}
  private def define_getter_and_setter(name, rules)
    T::Configuration.without_ruby_warnings do
      if !rules[:immutable]
        if method(:prop_set).owner != T::Props::Decorator
          @class.send(:define_method, "#{name}=") do |val|
            T.unsafe(self.class).decorator.prop_set(self, name, val, rules)
          end
        else
          # Fast path (~4x faster as of Ruby 2.6)
          @class.send(:define_method, "#{name}=", &rules.fetch(:setter_proc))
        end
      end

      if method(:prop_get).owner != T::Props::Decorator || rules.key?(:ifunset)
        @class.send(:define_method, name) do
          T.unsafe(self.class).decorator.prop_get(self, name, rules)
        end
      else
        # Fast path (~30x faster as of Ruby 2.6)
        @class.send(:attr_reader, name) # send is used because `attr_reader` is private in 2.4
      end
    end
  end

  # returns the subdoc of the array type, or nil if it's not a Document type
  #
  # checked(:never) - Typechecks internally
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
    .checked(:never)
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
  #
  # checked(:never) - Typechecks internally
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
    .checked(:never)
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

  # returns the type of the hash key, or nil. Any CustomType could be a key, but we only expect T::Enum right now.
  #
  # checked(:never) - Typechecks internally
  sig do
    params(type: PropType)
    .returns(T.nilable(Module))
    .checked(:never)
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
  TYPES_NOT_NEEDING_CLONE = T.let([TrueClass, FalseClass, NilClass, Symbol, String, Numeric], T::Array[Module])

  # checked(:never) - Typechecks internally
  sig {params(type: PropType).returns(T.nilable(T::Boolean)).checked(:never)}
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

  # checked(:never) - Rules hash is expensive to check
  sig {params(prop_name: Symbol, rules: Rules).void.checked(:never)}
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
      redaction: T.untyped,
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
      prop_name: Symbol,
      prop_cls: Module,
      foreign_hint_only: T.untyped,
    )
    .void
  end
  private def handle_foreign_hint_only_option(prop_name, prop_cls, foreign_hint_only)
    if ![String, Array].include?(prop_cls) && !(prop_cls.is_a?(T::Props::CustomType))
      raise ArgumentError.new(
        "`foreign_hint_only` can only be used with String or Array prop types"
      )
    end

    validate_foreign_option(
      :foreign_hint_only, foreign_hint_only,
      valid_type_msg: "an individual or array of a model class, or a Proc returning such."
    )

    unless foreign_hint_only.is_a?(Proc)
      T::Configuration.soft_assert_handler(<<~MESSAGE, storytime: {prop: prop_name, value: foreign_hint_only}, notify: 'jerry')
        Please use a Proc that returns a model class instead of the model class itself as the argument to `foreign_hint_only`. In other words:

          instead of `prop :foo, String, foreign_hint_only: FooModel`
          use `prop :foo, String, foreign_hint_only: -> {FooModel}`

          OR

          instead of `prop :foo, String, foreign_hint_only: [FooModel, BarModel]`
          use `prop :foo, String, foreign_hint_only: -> {[FooModel, BarModel]}`

      MESSAGE
    end
  end

  # checked(:never) - Rules hash is expensive to check
  sig do
    params(
      prop_name: T.any(String, Symbol),
      rules: Rules,
      foreign: T.untyped,
    )
    .void
    .checked(:never)
  end
  private def define_foreign_method(prop_name, rules, foreign)
    fk_method = "#{prop_name}_"

    # n.b. there's no clear reason *not* to allow additional options
    # here, but we're baking in `allow_direct_mutation` since we
    # *haven't* allowed additional options in the past and want to
    # default to keeping this interface narrow.
    @class.send(:define_method, fk_method) do |allow_direct_mutation: nil|
      foreign = T.let(foreign, T.untyped)
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

      T.unsafe(self.class).decorator.foreign_prop_get(self, prop_name, foreign, rules, opts)
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
      loaded_foreign
    end

    @class.send(:define_method, "#{prop_name}_record") do |allow_direct_mutation: nil|
      T::Configuration.soft_assert_handler(
        "Using deprecated 'model.#{prop_name}_record' foreign key syntax. You should replace this with 'model.#{prop_name}_'",
        notify: 'vasi'
      )
      send(fk_method, allow_direct_mutation: allow_direct_mutation)
    end
  end

  # checked(:never) - Rules hash is expensive to check
  sig do
    params(
      prop_name: Symbol,
      prop_cls: Module,
      rules: Rules,
      foreign: T.untyped,
    )
    .void
    .checked(:never)
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

    unless foreign.is_a?(Proc)
      T::Configuration.soft_assert_handler(<<~MESSAGE, storytime: {prop: prop_name, value: foreign}, notify: 'jerry')
        Please use a Proc that returns a model class instead of the model class itself as the argument to `foreign`. In other words:

          instead of `prop :foo, String, foreign: FooModel`
          use `prop :foo, String, foreign: -> {FooModel}`

      MESSAGE
    end

    define_foreign_method(prop_name, rules, foreign)
  end

  # TODO: rename this to props_inherited
  #
  # This gets called when a module or class that extends T::Props gets included, extended,
  # prepended, or inherited.
  sig {params(child: Module).void.checked(:never)}
  def model_inherited(child)
    child.extend(T::Props::ClassMethods)
    child = T.cast(child, T.all(Module, T::Props::ClassMethods))

    child.plugins.concat(decorated_class.plugins)
    decorated_class.plugins.each do |mod|
      # NB: apply_class_methods must not be an instance method on the decorator itself,
      # otherwise we'd have to call child.decorator here, which would create the decorator
      # before any `decorator_class` override has a chance to take effect (see the comment below).
      T::Props::Plugin::Private.apply_class_methods(mod, child)
    end

    props.each do |name, rules|
      copied_rules = rules.dup
      # NB: Calling `child.decorator` here is a timb bomb that's going to give someone a really bad
      # time. Any class that defines props and also overrides the `decorator_class` method is going
      # to reach this line before its override take effect, turning it into a no-op.
      child.decorator.add_prop_definition(name, copied_rules)

      # It's a bit tricky to support `prop_get` hooks added by plugins without
      # sacrificing the `attr_reader` fast path or clobbering customized getters
      # defined manually on a child.
      #
      # To make this work, we _do_ clobber getters defined on the child, but only if:
      # (a) it's needed in order to support a `prop_get` hook, and
      # (b) it's safe because the getter was defined by this file.
      #
      unless rules[:without_accessors]
        if child.decorator.method(:prop_get).owner != method(:prop_get).owner &&
            child.instance_method(name).source_location&.first == __FILE__
          child.send(:define_method, name) do
            T.unsafe(self.class).decorator.prop_get(self, name, rules)
          end
        end

        unless rules[:immutable]
          if child.decorator.method(:prop_set).owner != method(:prop_set).owner &&
              child.instance_method("#{name}=").source_location&.first == __FILE__
            child.send(:define_method, "#{name}=") do |val|
              T.unsafe(self.class).decorator.prop_set(self, name, val, rules)
            end
          end
        end
      end
    end
  end

  sig {params(mod: Module).void.checked(:never)}
  def plugin(mod)
    decorated_class.plugins << mod
    T::Props::Plugin::Private.apply_class_methods(mod, decorated_class)
    T::Props::Plugin::Private.apply_decorator_methods(mod, self)
  end
end
