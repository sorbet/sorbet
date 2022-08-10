# frozen_string_literal: true
# typed: false

module T::Private::Methods::CallValidation
  CallValidation = T::Private::Methods::CallValidation
  Modes = T::Private::Methods::Modes

  # Wraps a method with a layer of validation for the given type signature.
  # This wrapper is meant to be fast, and is applied by a previous wrapper,
  # which was placed by `_on_method_added`.
  #
  # @param method_sig [T::Private::Methods::Signature]
  # @return [UnboundMethod] the new wrapper method (or the original one if we didn't wrap it)
  def self.wrap_method_if_needed(mod, method_sig, original_method)
    original_visibility = visibility_method_name(mod, method_sig.method_name)
    if method_sig.mode == T::Private::Methods::Modes.abstract
      T::Private::ClassUtils.replace_method(mod, method_sig.method_name) do |*args, &blk|
        # TODO: write a cop to ensure that abstract methods have an empty body
        #
        # We allow abstract methods to be implemented by things further down the ancestor chain.
        # So, if a super method exists, call it.
        if defined?(super)
          super(*args, &blk)
        else
          raise NotImplementedError.new(
            "The method `#{method_sig.method_name}` on #{mod} is declared as `abstract`. It does not have an implementation."
          )
        end
      end
    # Do nothing in this case; this method was not wrapped in _on_method_added.
    elsif method_sig.defined_raw
    # Note, this logic is duplicated (intentionally, for micro-perf) at `Methods._on_method_added`,
    # make sure to keep changes in sync.
    # This is a trapdoor point for each method:
    # if a given method is wrapped, it stays wrapped; and if not, it's never wrapped.
    # (Therefore, we need the `@wrapped_tests_with_validation` check in `T::RuntimeLevels`.)
    elsif method_sig.check_level == :always || (method_sig.check_level == :tests && T::Private::RuntimeLevels.check_tests?)
      create_validator_method(mod, original_method, method_sig, original_visibility)
    else
      T::Configuration.without_ruby_warnings do
        # get all the shims out of the way and put back the original method
        T::Private::DeclState.current.without_on_method_added do
          T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility, original_method)
        end
      end
    end
    # Return the newly created method (or the original one if we didn't replace it)
    mod.instance_method(method_sig.method_name)
  end

  @is_allowed_to_have_fast_path = true
  def self.is_allowed_to_have_fast_path
    @is_allowed_to_have_fast_path
  end

  def self.disable_fast_path
    @is_allowed_to_have_fast_path = false
  end

  def self.create_validator_method(mod, original_method, method_sig, original_visibility)
    has_fixed_arity = method_sig.kwarg_types.empty? && !method_sig.has_rest && !method_sig.has_keyrest &&
      original_method.parameters.all? {|(kind, _name)| kind == :req}
    ok_for_fast_path = has_fixed_arity && !method_sig.bind && method_sig.arg_types.length < 5 && is_allowed_to_have_fast_path

    all_args_are_simple = ok_for_fast_path && method_sig.arg_types.all? {|_name, type| type.is_a?(T::Types::Simple)}
    simple_method = all_args_are_simple && method_sig.return_type.is_a?(T::Types::Simple)
    simple_procedure = all_args_are_simple && method_sig.return_type.is_a?(T::Private::Types::Void)

    T::Configuration.without_ruby_warnings do
      T::Private::DeclState.current.without_on_method_added do
        if simple_method
          create_validator_method_fast(mod, original_method, method_sig, original_visibility)
        elsif simple_procedure
          create_validator_procedure_fast(mod, original_method, method_sig, original_visibility)
        elsif ok_for_fast_path && method_sig.return_type.is_a?(T::Private::Types::Void)
          create_validator_procedure_medium(mod, original_method, method_sig, original_visibility)
        elsif ok_for_fast_path
          create_validator_method_medium(mod, original_method, method_sig, original_visibility)
        else
          create_validator_slow(mod, original_method, method_sig, original_visibility)
        end
      end
      mod.send(original_visibility, method_sig.method_name)
    end
  end

  def self.create_validator_slow(mod, original_method, method_sig, original_visibility)
    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |*args, &blk|
      CallValidation.validate_call(self, original_method, method_sig, args, blk)
    end
  end

  def self.validate_call(instance, original_method, method_sig, args, blk)
    # This method is called for every `sig`. It's critical to keep it fast and
    # reduce number of allocations that happen here.

    if method_sig.bind
      message = method_sig.bind.error_message_for_obj(instance)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Bind',
          nil,
          method_sig.bind,
          instance
        )
      end
    end

    # NOTE: We don't bother validating for missing or extra kwargs;
    # the method call itself will take care of that.
    method_sig.each_args_value_type(args) do |name, arg, type|
      message = type.error_message_for_obj(arg)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Parameter',
          name,
          type,
          arg,
          caller_offset: 2
        )
      end
    end

    if method_sig.block_type
      message = method_sig.block_type.error_message_for_obj(blk)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Block parameter',
          method_sig.block_name,
          method_sig.block_type,
          blk
        )
      end
    end

    # The following line breaks are intentional to show nice pry message










    # PRY note:
    # this code is sig validation code.
    # Please issue `finish` to step out of it

    return_value = T::Configuration::AT_LEAST_RUBY_2_7 ? original_method.bind_call(instance, *args, &blk) : original_method.bind(instance).call(*args, &blk)

    # The only type that is allowed to change the return value is `.void`.
    # It ignores what you returned and changes it to be a private singleton.
    if method_sig.return_type.is_a?(T::Private::Types::Void)
      T::Private::Types::Void::VOID
    else
      message = method_sig.return_type.error_message_for_obj(return_value)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Return value',
          nil,
          method_sig.return_type,
          return_value,
        )
      end
      return_value
    end
  end

  def self.report_error(method_sig, error_message, kind, name, type, value, caller_offset: 0)
    caller_loc = T.must(caller_locations(3 + caller_offset, 1))[0]
    definition_file, definition_line = method_sig.method.source_location

    pretty_message = "#{kind}#{name ? " '#{name}'" : ''}: #{error_message}\n" \
      "Caller: #{caller_loc.path}:#{caller_loc.lineno}\n" \
      "Definition: #{definition_file}:#{definition_line}"

    T::Configuration.call_validation_error_handler(
      method_sig,
      message: error_message,
      pretty_message: pretty_message,
      kind: kind,
      name: name,
      type: type,
      value: value,
      location: caller_loc
    )
  end

  # `name` must be an instance method (for class methods, pass in mod.singleton_class)
  private_class_method def self.visibility_method_name(mod, name)
    if mod.public_method_defined?(name)
      :public
    elsif mod.protected_method_defined?(name)
      :protected
    elsif mod.private_method_defined?(name)
      :private
    else
      raise NameError.new("undefined method `#{name}` for `#{mod}`")
    end
  end
end

if T::Configuration::AT_LEAST_RUBY_2_7
  require_relative './call_validation_2_7'
else
  require_relative './call_validation_2_6'
end
