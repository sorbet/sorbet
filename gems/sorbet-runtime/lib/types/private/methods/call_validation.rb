# frozen_string_literal: true
# typed: true

module T::Private::Methods::CallValidation
  CallValidation = T::Private::Methods::CallValidation
  Modes = T::Private::Methods::Modes

  KERNEL_TO_S = Kernel.instance_method(:to_s)
  MODULE_TO_S = Module.instance_method(:to_s)
  private_constant(:KERNEL_TO_S, :MODULE_TO_S)

  # Wraps a method with a layer of validation for the given type signature.
  # This wrapper is meant to be fast, and is applied by a previous wrapper,
  # which was placed by `_on_method_added`.
  #
  # @param method_sig [T::Private::Methods::Signature]
  # @return [UnboundMethod] the new wrapper method (or the original one if we didn't wrap it)
  def self.wrap_method_if_needed(mod, method_sig, original_method)
    original_visibility = T::Private::ClassUtils.visibility_method_name(mod, method_sig.method_name)
    if method_sig.mode == T::Private::Methods::Modes.abstract
      create_abstract_wrapper(mod, method_sig.method_name, original_visibility)
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

  def self.create_abstract_wrapper(mod, method_name, original_visibility)
    T::Configuration.without_ruby_warnings do
      T::Private::DeclState.current.without_on_method_added do
        mod.module_eval(<<~METHOD, __FILE__, __LINE__ + 1)
          #{original_visibility}

          def #{method_name}(...)
            # We allow abstract methods to be implemented by things further down the ancestor chain.
            # So, if a super method exists, call it.
            if defined?(super)
              super
            else
              raise NotImplementedError.new(
                "The method `#{method_name}` on #{mod} is declared as `abstract`. It does not have an implementation."
              )
            end
          end
        METHOD
      end
    end
  end

  def self.create_validator_method(mod, original_method, method_sig, original_visibility)
    # `method_sig.parameters` was saved off of `original_method.parameters` when
    # the sig was built; reusing it avoids re-allocating the parameters list.
    parameters = method_sig.parameters
    has_fixed_arity = method_sig.kwarg_types.empty? && method_sig.rest_type.nil? && method_sig.keyrest_type.nil? &&
      parameters.all? { |(kind, _name)| kind == :req || kind == :block }

    # nil implies block_type.nil?
    # true implies !block_type.nil? and block_type.valid?(nil)
    # false implies !block_type.nil? and !block_type.valid?(nil)
    # This formulation avoids a type error without introducing extra method calls or local vars
    can_skip_block_type = method_sig.block_type&.valid?(nil) != false

    ok_for_fast_path = has_fixed_arity && can_skip_block_type && !method_sig.bind && method_sig.arg_types.length < 7 && is_allowed_to_have_fast_path

    # Like `ok_for_fast_path`, but for the specialized wrappers below, each of
    # which supports exactly one extra call shape (kwargs, a required block, or
    # optional positional args) on top of what the fast/medium wrappers support.
    ok_for_specialized_path = !ok_for_fast_path && !method_sig.bind && method_sig.rest_type.nil? &&
      method_sig.keyrest_type.nil? && method_sig.arg_types.length < 7 && is_allowed_to_have_fast_path

    kwargs_path = ok_for_specialized_path && can_skip_block_type && !method_sig.kwarg_types.empty? &&
      parameters.all? { |(kind, _name)| kind == :req || kind == :key || kind == :keyreq || kind == :block }

    required_block_path = ok_for_specialized_path && !can_skip_block_type && has_fixed_arity

    # At least one param is `:opt` whenever this is true (otherwise
    # `ok_for_fast_path` would have been true instead).
    optional_args_path = ok_for_specialized_path && can_skip_block_type && method_sig.kwarg_types.empty? &&
      parameters.all? { |(kind, _name)| kind == :req || kind == :opt || kind == :block }

    all_args_are_simple = ok_for_fast_path && method_sig.arg_types.all? { |_name, type| type.is_a?(T::Types::Simple) }

    effective_return_type = method_sig.effective_return_type
    simple_method = all_args_are_simple && effective_return_type.is_a?(T::Types::Simple)
    simple_procedure = all_args_are_simple && effective_return_type.is_a?(T::Private::Types::Void)

    # All the types for which valid? unconditionally returns `true`
    return_is_ignorable =
      effective_return_type.equal?(T::Types::Untyped::Private::INSTANCE) ||
      effective_return_type.equal?(T::Types::Anything::Private::INSTANCE) ||
      effective_return_type.equal?(T::Types::AttachedClassType::Private::INSTANCE) ||
      effective_return_type.equal?(T::Types::SelfType::Private::INSTANCE) ||
      effective_return_type.is_a?(T::Types::TypeParameter) ||
      effective_return_type.is_a?(T::Types::TypeVariable) ||
      (effective_return_type.is_a?(T::Types::Simple) && effective_return_type.raw_type.equal?(BasicObject))

    returns_anything_method = all_args_are_simple && return_is_ignorable

    T::Configuration.without_ruby_warnings do
      T::Private::DeclState.current.without_on_method_added do
        if simple_method
          create_validator_method_fast(mod, original_method, method_sig, original_visibility)
        elsif returns_anything_method
          create_validator_method_skip_return_fast(mod, original_method, method_sig, original_visibility)
        elsif simple_procedure
          create_validator_procedure_fast(mod, original_method, method_sig, original_visibility)
        elsif ok_for_fast_path && effective_return_type.is_a?(T::Private::Types::Void)
          create_validator_procedure_medium(mod, original_method, method_sig, original_visibility)
        elsif ok_for_fast_path && return_is_ignorable
          create_validator_method_skip_return_medium(mod, original_method, method_sig, original_visibility)
        elsif ok_for_fast_path
          create_validator_method_medium(mod, original_method, method_sig, original_visibility)
        elsif kwargs_path
          create_validator_method_kwargs(mod, original_method, method_sig, original_visibility)
        elsif required_block_path
          create_validator_method_with_block(mod, original_method, method_sig, original_visibility)
        elsif optional_args_path
          create_validator_method_optional_args(mod, original_method, method_sig, original_visibility)
        elsif can_skip_block_type
          # The Ruby VM already validates that any block passed to a method
          # must be either `nil` or a `Proc` object, so there's no need to also
          # have sorbet-runtime check that.
          create_validator_slow_skip_block_type(mod, original_method, method_sig, original_visibility)
        else
          create_validator_slow(mod, original_method, method_sig, original_visibility)
        end
      end
      mod.send(original_visibility, method_sig.method_name)
    end
  end

  # Sentinel for an optional positional arg that was not provided at the call
  # site. A private module can never collide with a legitimate argument value.
  # (Same approach as `T::Private::Methods::ARG_NOT_PROVIDED`.)
  ARG_NOT_PROVIDED = Module.new.freeze
  private_constant(:ARG_NOT_PROVIDED)

  def self.create_validator_method_kwargs(mod, original_method, method_sig, original_visibility)
    # `nil` for `return_type` means the method is `.void` (the wrapper then
    # returns `T::Private::Types::Void::VOID` without validating the return).
    return_type = method_sig.effective_return_type
    return_type = nil if return_type.is_a?(T::Private::Types::Void)
    kwarg_types = method_sig.kwarg_types
    # trampoline to reduce stack frame size
    arg_types = method_sig.arg_types
    case arg_types.length
    when 0
      create_validator_method_kwargs0(mod, original_method, method_sig, original_visibility, return_type, kwarg_types)
    when 1
      create_validator_method_kwargs1(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1])
    when 2
      create_validator_method_kwargs2(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1],
                                    arg_types[1][1])
    when 3
      create_validator_method_kwargs3(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1])
    when 4
      create_validator_method_kwargs4(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1])
    when 5
      create_validator_method_kwargs5(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1],
                                    arg_types[4][1])
    when 6
      create_validator_method_kwargs6(mod, original_method, method_sig, original_visibility, return_type, kwarg_types,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1],
                                    arg_types[4][1],
                                    arg_types[5][1])
    else
      raise 'should not happen'
    end
  end

  def self.create_validator_method_with_block(mod, original_method, method_sig, original_visibility)
    # `nil` for `return_type` means the method is `.void` (the wrapper then
    # returns `T::Private::Types::Void::VOID` without validating the return).
    return_type = method_sig.effective_return_type
    return_type = nil if return_type.is_a?(T::Private::Types::Void)
    block_type = method_sig.block_type
    # trampoline to reduce stack frame size
    arg_types = method_sig.arg_types
    case arg_types.length
    when 0
      create_validator_method_with_block0(mod, original_method, method_sig, original_visibility, return_type, block_type)
    when 1
      create_validator_method_with_block1(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1])
    when 2
      create_validator_method_with_block2(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1],
                                    arg_types[1][1])
    when 3
      create_validator_method_with_block3(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1])
    when 4
      create_validator_method_with_block4(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1])
    when 5
      create_validator_method_with_block5(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1],
                                    arg_types[4][1])
    when 6
      create_validator_method_with_block6(mod, original_method, method_sig, original_visibility, return_type, block_type,
                                    arg_types[0][1],
                                    arg_types[1][1],
                                    arg_types[2][1],
                                    arg_types[3][1],
                                    arg_types[4][1],
                                    arg_types[5][1])
    else
      raise 'should not happen'
    end
  end

  def self.create_validator_method_optional_args(mod, original_method, method_sig, original_visibility)
    # `nil` for `return_type` means the method is `.void` (the wrapper then
    # returns `T::Private::Types::Void::VOID` without validating the return).
    return_type = method_sig.effective_return_type
    return_type = nil if return_type.is_a?(T::Private::Types::Void)
    # trampoline to reduce stack frame size
    arg_types = method_sig.arg_types
    arg_count = arg_types.length
    if arg_count > 6 || method_sig.req_arg_count >= arg_count
      raise 'should not happen'
    end
    send(
      :"create_validator_method_optional_args#{method_sig.req_arg_count}_#{arg_count}",
      mod, original_method, method_sig, original_visibility, return_type,
      *arg_types.map { |_name, type| type }
    )
  end

  # The wrappers below are unrolled per arity, in the same style as the
  # generated wrappers in `call_validation_2_7.rb`, but cover three call shapes
  # that file does not: keyword args, required (non-nilable) blocks, and
  # optional positional args. They are `module_eval`ed at load time instead of
  # being checked in, to avoid maintaining many near-identical copies by hand.

  # Generates one `unless argN_type.valid?(argN) ... end` check, matching the
  # positional arg checks in the `call_validation_2_7.rb` medium wrappers.
  arg_check = lambda do |i|
    <<~RUBY
      unless arg#{i}_type.valid?(arg#{i})
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[#{i}][1].error_message_for_obj(arg#{i}),
          'Parameter',
          method_sig.arg_types[#{i}][0],
          arg#{i}_type,
          arg#{i},
          caller_offset: -1
        )
      end
    RUBY
  end

  # Matches the return handling in the `call_validation_2_7.rb` medium
  # wrappers, except that `nil` for `return_type` means `.void`.
  return_tail = <<~RUBY
    if return_type.nil?
      T::Private::Types::Void::VOID
    else
      unless return_type.valid?(return_value)
        message = method_sig.effective_return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            method_sig.effective_return_type,
            return_value,
            caller_offset: -1
          )
        end
      end
      return_value
    end
  RUBY

  (0..6).each do |arity|
    args = (0...arity).map { |i| "arg#{i}, " }.join
    type_params = (0...arity).map { |i| ", arg#{i}_type" }.join
    arg_checks = (0...arity).map { |i| arg_check.call(i) }.join

    module_eval(<<~RUBY, __FILE__, __LINE__ + 1)
      def self.create_validator_method_kwargs#{arity}(mod, original_method, method_sig, original_visibility, return_type, kwarg_types#{type_params})
        T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |#{args}**kwargs, &blk|
          # This method is a manually sped-up version of more general code in `validate_call`
          #{arg_checks}
          # NOTE: like `validate_call`, we don't validate for missing or extra
          # kwargs; the `bind_call` below takes care of that.
          kwargs.each do |name, val|
            type = kwarg_types[name]
            next unless type
            unless type.valid?(val)
              CallValidation.report_error(
                method_sig,
                type.error_message_for_obj(val),
                'Parameter',
                name,
                type,
                val,
                caller_offset: 1
              )
            end
          end
          return_value = original_method.bind_call(self, #{args}**kwargs, &blk)
          #{return_tail}
        end
      end

      def self.create_validator_method_with_block#{arity}(mod, original_method, method_sig, original_visibility, return_type, block_type#{type_params})
        T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |#{args}&blk|
          # This method is a manually sped-up version of more general code in `validate_call`
          #{arg_checks}
          if blk.nil?
            CallValidation.report_error(
              method_sig,
              block_type.error_message_for_obj(blk),
              'Block parameter',
              method_sig.block_name,
              block_type,
              blk,
              caller_offset: -1
            )
          end
          return_value = original_method.bind_call(self, #{args}&blk)
          #{return_tail}
        end
      end
    RUBY
  end

  (1..6).each do |total|
    (0...total).each do |req|
      params = (0...req).map { |i| "arg#{i}, " }.join +
        (req...total).map { |i| "arg#{i} = ARG_NOT_PROVIDED, " }.join
      type_params = (0...total).map { |i| ", arg#{i}_type" }.join
      req_checks = (0...req).map { |i| arg_check.call(i) }.join

      # Cascade on the first not-provided optional arg: validate the args that
      # were actually provided (defaults are never validated, same as the slow
      # path), then forward exactly those args so the original method computes
      # the defaults for the rest.
      dispatch = +""
      (req..total).each do |provided|
        opt_checks = (req...provided).map { |i| arg_check.call(i) }.join
        call_args = (0...provided).map { |i| "arg#{i}, " }.join
        body = "#{opt_checks}return_value = original_method.bind_call(self, #{call_args}&blk)\n"
        dispatch <<
          if provided == req
            "if ARG_NOT_PROVIDED.equal?(arg#{provided})\n#{body}"
          elsif provided == total
            "else\n#{body}end\n"
          else
            "elsif ARG_NOT_PROVIDED.equal?(arg#{provided})\n#{body}"
          end
      end

      module_eval(<<~RUBY, __FILE__, __LINE__ + 1)
        def self.create_validator_method_optional_args#{req}_#{total}(mod, original_method, method_sig, original_visibility, return_type#{type_params})
          T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |#{params}&blk|
            # This method is a manually sped-up version of more general code in `validate_call`
            #{req_checks}
            #{dispatch}
            #{return_tail}
          end
        end
      RUBY
    end
  end

  def self.create_validator_slow_skip_block_type(mod, original_method, method_sig, original_visibility)
    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |*args, &blk|
      CallValidation.validate_call_skip_block_type(self, original_method, method_sig, args, blk)
    end
  end

  def self.validate_call_skip_block_type(instance, original_method, method_sig, args, blk)
    # This method is called for every `sig`. It's critical to keep it fast and
    # reduce number of allocations that happen here.

    if method_sig.bind
      message = method_sig.bind&.error_message_for_obj(instance)
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

    # The original method definition allows passing `nil` for the `&blk`
    # argument, so we do not have to do any method_sig.block_type type checks
    # of our own.

    # The following line breaks are intentional to show nice pry message










    # PRY note:
    # this code is sig validation code.
    # Please issue `finish` to step out of it

    return_value = original_method.bind_call(instance, *args, &blk)

    # The only type that is allowed to change the return value is `.void`.
    # It ignores what you returned and changes it to be a private singleton.
    if method_sig.effective_return_type.is_a?(T::Private::Types::Void)
      T::Private::Types::Void::VOID
    else
      message = method_sig.effective_return_type.error_message_for_obj(return_value)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Return value',
          nil,
          method_sig.effective_return_type,
          return_value,
        )
      end
      return_value
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

    # The Ruby VM already checks that `&blk` is either a `Proc` type or `nil`:
    # https://github.com/ruby/ruby/blob/v2_7_6/vm_args.c#L1150-L1154
    # And `T.proc` types don't (can't) do any runtime arg checking, so we can
    # save work by simply checking that `blk` is non-nil (if the method allows
    # `nil` for the block, it would not have used this validate_call path).
    unless blk
      # Have to use `&.` here, because it's technically a public API that
      # people can _always_ call `validate_call` to validate any signature
      # (i.e., the faster validators are merely optimizations).
      # In practice, this only affects the first call to the method (before the
      # optimized validators have a chance to replace the initial, slow
      # wrapper).
      message = method_sig.block_type&.error_message_for_obj(blk)
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

    return_value = original_method.bind_call(instance, *args, &blk)

    # The only type that is allowed to change the return value is `.void`.
    # It ignores what you returned and changes it to be a private singleton.
    if method_sig.effective_return_type.is_a?(T::Private::Types::Void)
      T::Private::Types::Void::VOID
    else
      message = method_sig.effective_return_type.error_message_for_obj(return_value)
      if message
        CallValidation.report_error(
          method_sig,
          message,
          'Return value',
          nil,
          method_sig.effective_return_type,
          return_value,
        )
      end
      return_value
    end
  end

  # Get the name of a method owner via its `.to_s`, but fallback if its implementation raises or returns `nil`.
  private_class_method def self.safe_method_owner_to_s(method_owner)
    case method_owner
    when Module # methods are usually owned by a Class or Module...
      MODULE_TO_S.bind_call(method_owner)
    else # ... but could be a singleton method on any kind of Object.
      KERNEL_TO_S.bind_call(method_owner)
    end
  end

  def self.report_error(method_sig, error_message, kind, name, type, value, caller_offset: 0)
    caller_loc = T.must(caller_locations(3 + caller_offset, 1))[0]
    method = method_sig.method
    definition_file, definition_line = method.source_location

    owner = method.owner
    pretty_method_name =
      if owner.singleton_class? && owner.respond_to?(:attached_object)
        # attached_object is new in Ruby 3.2
        "#{safe_method_owner_to_s(owner.attached_object)}.#{method.name}"
      else
        "#{safe_method_owner_to_s(owner)}##{method.name}"
      end

    pretty_message = "#{kind}#{name ? " '#{name}'" : ''}: #{error_message}\n" \
      "Caller: #{caller_loc&.path}:#{caller_loc&.lineno}\n" \
      "Definition: #{definition_file}:#{definition_line} (#{pretty_method_name})"

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
end

require_relative './call_validation_2_7'
