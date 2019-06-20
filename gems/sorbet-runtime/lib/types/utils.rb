# frozen_string_literal: true
# typed: true

module T::Utils
  # Used to convert from a type specification to a `T::Types::Base`.
  def self.coerce(val)
    if val.is_a?(T::Types::Base)
      val
    elsif val == ::Array
      T::Array[T.untyped]
    elsif val == ::Set
      T::Set[T.untyped]
    elsif val == ::Hash
      T::Hash[T.untyped, T.untyped]
    elsif val == ::Enumerable
      T::Enumerable[T.untyped]
    elsif val == ::Enumerator
      T::Enumerator[T.untyped]
    elsif val == ::Range
      T::Range[T.untyped]
    elsif val.is_a?(Module)
      T::Types::Simple.new(val) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
    elsif val.is_a?(::Array)
      T::Types::FixedArray.new(val) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
    elsif val.is_a?(::Hash)
      T::Types::FixedHash.new(val) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
    elsif val.is_a?(T::Private::Methods::DeclBuilder)
      T::Private::Methods.finalize_proc(val.decl)
    else
      raise "Invalid value for type constraint. Must be an #{T::Types::Base}, a " \
            "class/module, or an array. Got a `#{val.class}`."
    end
  end

  # Returns the set of all methods (public, protected, private) defined on a module or its
  # ancestors, excluding Object and its ancestors. Overrides of methods from Object (and its
  # ancestors) are included.
  def self.methods_excluding_object(mod)
    # We can't just do mod.instance_methods - Object.instance_methods, because that would leave out
    # any methods from Object that are overridden in mod.
    mod.ancestors.flat_map do |ancestor|
      # equivalent to checking Object.ancestors.include?(ancestor)
      next [] if Object <= ancestor
      ancestor.instance_methods(false) + ancestor.private_instance_methods(false)
    end.uniq
  end

  # Associates a signature with a forwarder method that matches the signature of the method it
  # forwards to. This is necessary because forwarder methods are often declared with catch-all
  # splat parameters, rather than the exact set of parameters ultimately used by the target method,
  # so they cannot be validated as strictly.
  #
  # The caller of this method must ensure that the forwarder method properly forwards all parameters
  # such that the signature is accurate.
  def self.register_forwarder(from_method, to_method, remove_first_param: false)
    T::Private::Methods.register_forwarder(
      from_method, to_method, remove_first_param: remove_first_param
    )
  end

  # Returns the signature for the instance method on the supplied module, or nil if it's not found or not typed.
  #
  # @example T::Utils.signature_for_instance_method(MyClass, :my_method)
  def self.signature_for_instance_method(mod, method_name)
    T::Private::Methods.signature_for_method(mod.instance_method(method_name))
  end

  def self.wrap_method_with_call_validation_if_needed(mod, method_sig, original_method)
    T::Private::Methods::CallValidation.wrap_method_if_needed(mod, method_sig, original_method)
  end

  # Unwraps all the sigs.
  def self.run_all_sig_blocks
    T::Private::Methods.run_all_sig_blocks
  end

  # This can be called in a wholesome test to make sure all the `sig`s are well
  # formed.
  def self.validate_sigs
    exceptions = []
    run_all_sig_blocks
    ObjectSpace.each_object(Module) do |mod| # rubocop:disable PrisonGuard/NoDynamicConstAccess
      begin
        T::Private::Abstract::Validate.validate_subclass(mod)
        if T::AbstractUtils.abstract_module?(mod)
          T::Private::Abstract::Validate.validate_abstract_module(mod)
        end
      rescue => e
        exceptions << e
      end
    end
    if !exceptions.empty?
      raise "#{exceptions.count} exception thrown during validation:\n\n#{exceptions.map(&:message).sort.join("\n\n")}"
    end
  end

  # Give a type which is a subclass of T::Types::Base, determines if the type is a simple nilable type (union of NilClass and something else).
  # If so, returns the T::Types::Base of the something else. Otherwise, returns nil.
  def self.unwrap_nilable(type)
    case type
    when T::Types::Union
      non_nil_types = type.types.reject {|t| t == T::Utils.coerce(NilClass)}
      if non_nil_types.length == 1
        non_nil_types.first
      else
        nil
      end
    else
      nil
    end
  end

  def self.DANGER_enable_checking_in_tests
    T::Private::RuntimeLevels.enable_checking_in_tests
  end

  # Returns the arity of a method, unwrapping the sig if needed
  def self.arity(method)
    arity = method.arity # rubocop:disable PrisonGuard/NoArity
    return arity if arity != -1 || method.is_a?(Proc)
    sig = T::Private::Methods.signature_for_method(method)
    sig ? sig.method.arity : arity # rubocop:disable PrisonGuard/NoArity
  end

  # Elide the middle of a string as needed and replace it with an ellipsis.
  # Keep the given number of characters at the start and end of the string.
  #
  # This method operates on string length, not byte length.
  #
  # If the string is shorter than the requested truncation length, return it
  # without adding an ellipsis. This method may return a longer string than
  # the original if the characters removed are shorter than the ellipsis.
  #
  # @param [String] str
  #
  # @param [Fixnum] start_len The length of string before the ellipsis
  # @param [Fixnum] end_len The length of string after the ellipsis
  #
  # @param [String] ellipsis The string to add in place of the elided text
  #
  # @return [String]
  #
  def self.string_truncate_middle(str, start_len, end_len, ellipsis='...')
    return unless str

    raise ArgumentError.new('must provide start_len') unless start_len
    raise ArgumentError.new('must provide end_len') unless end_len

    raise ArgumentError.new('start_len must be >= 0') if start_len < 0
    raise ArgumentError.new('end_len must be >= 0') if end_len < 0

    str = str.to_s
    return str if str.length <= start_len + end_len

    start_part = str[0...start_len - ellipsis.length]
    end_part = end_len == 0 ? '' : str[-end_len..-1]

    "#{start_part}#{ellipsis}#{end_part}"
  end

  module Props
    def self.required_prop?(prop_rules)
      # Clients should never reference :_tnilable as the implementation can change.
      !prop_rules[:_tnilable]
    end

    def self.optional_prop?(prop_rules)
      # Clients should never reference :_tnilable as the implementation can change.
      !!prop_rules[:_tnilable]
    end

    def self.merge_serialized_optional_rule(prop_rules)
      {'_tnilable' => true}.merge(prop_rules.merge('_tnilable' => true))
    end
  end

  module Nilable
    # :is_union_type, T::Boolean: whether the type is an T::Types::Union type
    # :non_nilable_type, Class: if it is an T.nilable type, the corresponding underlying type; otherwise, nil.
    TypeInfo = Struct.new(:is_union_type, :non_nilable_type)

    def self.get_type_info(prop_type)
      if prop_type.is_a?(T::Types::Union)
        non_nilable_type = T::Utils.unwrap_nilable(prop_type)
        if non_nilable_type && non_nilable_type.is_a?(T::Types::Simple)
          non_nilable_type = non_nilable_type.raw_type
        end
        TypeInfo.new(true, non_nilable_type)
      else
        TypeInfo.new(false, nil)
      end
    end

    # Get the underlying type inside prop_type:
    #  - if the type is A, the function returns A
    #  - if the type is T.nilable(A), the function returns A
    def self.get_underlying_type(prop_type)
      type_info = get_type_info(prop_type)
      if type_info.is_union_type
        type_info.non_nilable_type || prop_type
      elsif prop_type.is_a?(T::Types::Simple)
        prop_type.raw_type
      else
        prop_type
      end
    end

    # The difference between this function and the above function is that the Sorbet type, like T::Types::Simple
    # is preserved.
    def self.get_underlying_type_object(prop_type)
      T::Utils.unwrap_nilable(prop_type) || prop_type
    end

    def self.is_union_with_nilclass(prop_type)
      case prop_type
      when T::Types::Union
        prop_type.types.any? {|t| t == T::Utils.coerce(NilClass)}
      else
        false
      end
    end
  end
end
