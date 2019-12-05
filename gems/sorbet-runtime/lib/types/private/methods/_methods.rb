# frozen_string_literal: true
# typed: false

module T::Private::Methods
  @installed_hooks = Set.new
  @signatures_by_method = {}
  @sig_wrappers = {}
  @sigs_that_raised = {}
  # the info about whether a method is final is not stored in a DeclBuilder nor a Signature, but instead right here.
  # this is because final checks are special:
  # - they are done possibly before any sig block has run.
  # - they are done even if the method being defined doesn't have a sig.
  @final_methods = Set.new
  # a non-singleton is a module for which at least one of the following is true:
  # - is declared final
  # - defines a method that is declared final
  # - includes an non-singleton
  # - extends an non-singleton
  # a singleton is the singleton_class of a non-singleton.
  # modules_with_final is the set of singletons and non-singletons.
  @modules_with_final = Set.new
  # this stores the old [included, extended] hooks for Module and inherited hook for Class that we override when
  # enabling final checks for when those hooks are called. the 'hooks' here don't have anything to do with the 'hooks'
  # in installed_hooks.
  @old_hooks = nil

  ARG_NOT_PROVIDED = Object.new
  PROC_TYPE = Object.new

  DeclarationBlock = Struct.new(:mod, :loc, :blk, :final)

  def self.declare_sig(mod, arg, &blk)
    install_hooks(mod)

    if T::Private::DeclState.current.active_declaration
      T::Private::DeclState.current.reset!
      raise "You called sig twice without declaring a method inbetween"
    end

    if !arg.nil? && arg != :final
      raise "Invalid argument to `sig`: #{arg}"
    end

    loc = caller_locations(2, 1).first

    T::Private::DeclState.current.active_declaration = DeclarationBlock.new(mod, loc, blk, arg == :final)

    nil
  end

  def self.start_proc
    DeclBuilder.new(PROC_TYPE)
  end

  def self.finalize_proc(decl)
    decl.finalized = true

    if decl.mode != Modes.standard
      raise "Procs cannot have override/abstract modifiers"
    end
    if decl.mod != PROC_TYPE
      raise "You are passing a DeclBuilder as a type. Did you accidentally use `self` inside a `sig` block?"
    end
    if decl.returns == ARG_NOT_PROVIDED
      raise "Procs must specify a return type"
    end
    if decl.on_failure != ARG_NOT_PROVIDED
      raise "Procs cannot use .on_failure"
    end

    if decl.params == ARG_NOT_PROVIDED
      decl.params = {}
    end

    T::Types::Proc.new(decl.params, decl.returns) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
  end

  # See docs at T::Utils.register_forwarder.
  def self.register_forwarder(from_method, to_method, mode: Modes.overridable, remove_first_param: false)
    # Normalize the method (see comment in signature_for_key).
    from_method = from_method.owner.instance_method(from_method.name)

    from_key = method_to_key(from_method)
    maybe_run_sig_block_for_key(from_key)
    if @signatures_by_method.key?(from_key)
      raise "#{from_method} already has a method signature"
    end

    from_params = from_method.parameters
    if from_params.length != 2 || from_params[0][0] != :rest || from_params[1][0] != :block
      raise "forwarder methods should take a single splat param and a block param. `#{from_method}` " \
            "takes these params: #{from_params}. For help, ask #dev-productivity."
    end

    # If there's already a signature for to_method, we get `parameters` from there, to enable
    # chained forwarding. NB: we don't use `signature_for_key` here, because the normalization it
    # does is broken when `to_method` has been clobbered by another method.
    to_key = method_to_key(to_method)
    maybe_run_sig_block_for_key(to_key)
    to_params = @signatures_by_method[to_key]&.parameters || to_method.parameters

    if remove_first_param
      to_params = to_params[1..-1]
    end

    # We don't bother trying to preserve any types from to_signature because this won't be
    # statically analyzed, and the types will just get checked when the forwarding happens.
    from_signature = Signature.new_untyped(method: from_method, mode: mode, parameters: to_params)
    @signatures_by_method[from_key] = from_signature
  end

  # Returns the signature for a method whose definition was preceded by `sig`.
  #
  # @param method [UnboundMethod]
  # @return [T::Private::Methods::Signature]
  def self.signature_for_method(method)
    signature_for_key(method_to_key(method))
  end

  private_class_method def self.signature_for_key(key)
    maybe_run_sig_block_for_key(key)

    # If a subclass Sub inherits a method `foo` from Base, then
    # Sub.instance_method(:foo) != Base.instance_method(:foo) even though they resolve to the
    # same method. Similarly, Foo.method(:bar) != Foo.singleton_class.instance_method(:bar).
    # So, we always do the look up by the method on the owner (Base in this example).
    @signatures_by_method[key]
  end

  # when target includes a module with instance methods source_method_names, ensure there is zero intersection between
  # the final instance methods of target and source_method_names. so, for every m in source_method_names, check if there
  # is already a method defined on one of target_ancestors with the same name that is final.
  def self._check_final_ancestors(target, target_ancestors, source_method_names)
    if !module_with_final?(target)
      return
    end
    # use reverse_each to check farther-up ancestors first, for better error messages. we could avoid this if we were on
    # the version of ruby that adds the optional argument to method_defined? that allows you to exclude ancestors.
    target_ancestors.reverse_each do |ancestor|
      source_method_names.each do |method_name|
        # the usage of method_owner_and_name_to_key(ancestor, method_name) instead of
        # method_to_key(ancestor.instance_method(method_name)) is not (just) an optimization, but also required for
        # correctness, since ancestor.method_defined?(method_name) may return true even if method_name is not defined
        # directly on ancestor but instead an ancestor of ancestor.
        if ancestor.method_defined?(method_name) && final_method?(method_owner_and_name_to_key(ancestor, method_name))
          raise(
            "The method `#{method_name}` on #{ancestor} was declared as final and cannot be " +
            (target == ancestor ? "redefined" : "overridden in #{target}")
          )
        end
      end
    end
  end

  private_class_method def self.add_final_method(method_key)
    @final_methods.add(method_key)
  end

  private_class_method def self.final_method?(method_key)
    @final_methods.include?(method_key)
  end

  def self.add_module_with_final(mod)
    @modules_with_final.add(mod)
    @modules_with_final.add(mod.singleton_class)
  end

  private_class_method def self.module_with_final?(mod)
    @modules_with_final.include?(mod)
  end

  # Only public because it needs to get called below inside the replace_method blocks below.
  def self._on_method_added(hook_mod, method_name, is_singleton_method: false)
    if T::Private::DeclState.current.skip_on_method_added
      return
    end

    current_declaration = T::Private::DeclState.current.active_declaration
    mod = is_singleton_method ? hook_mod.singleton_class : hook_mod

    if T::Private::Final.final_module?(mod) && (current_declaration.nil? || !current_declaration.final)
      raise "#{mod} was declared as final but its method `#{method_name}` was not declared as final"
    end
    _check_final_ancestors(mod, mod.ancestors, [method_name])

    if current_declaration.nil?
      return
    end
    T::Private::DeclState.current.reset!

    if method_name == :method_added || method_name == :singleton_method_added
      raise(
        "Putting a `sig` on `#{method_name}` is not supported" +
        " (sorbet-runtime uses this method internally to perform `sig` validation logic)"
      )
    end

    original_method = mod.instance_method(method_name)
    sig_block = lambda do
      T::Private::Methods.run_sig(hook_mod, method_name, original_method, current_declaration)
    end

    # Always replace the original method with this wrapper,
    # which is called only on the *first* invocation.
    # This wrapper is very slow, so it will subsequently re-wrap with a much faster wrapper
    # (or unwrap back to the original method).
    new_method = nil
    T::Private::ClassUtils.replace_method(mod, method_name) do |*args, &blk|
      if !T::Private::Methods.has_sig_block_for_method(new_method)
        # This should only happen if the user used alias_method to grab a handle
        # to the original pre-unwound `sig` method. I guess we'll just proxy the
        # call forever since we don't know who is holding onto this handle to
        # replace it.
        new_new_method = mod.instance_method(method_name)
        if new_method == new_new_method
          raise "`sig` not present for method `#{method_name}` but you're trying to run it anyways. " \
          "This should only be executed if you used `alias_method` to grab a handle to a method after `sig`ing it, but that clearly isn't what you are doing. " \
          "Maybe look to see if an exception was thrown in your `sig` lambda or somehow else your `sig` wasn't actually applied to the method. " \
          "Contact #dev-productivity if you're really stuck."
        end
        return new_new_method.bind(self).call(*args, &blk)
      end

      method_sig = T::Private::Methods.run_sig_block_for_method(new_method)

      # Should be the same logic as CallValidation.wrap_method_if_needed but we
      # don't want that extra layer of indirection in the callstack
      if method_sig.mode == T::Private::Methods::Modes.abstract
        # We're in an interface method, keep going up the chain
        if defined?(super)
          super(*args, &blk)
        else
          raise NotImplementedError.new("The method `#{method_sig.method_name}` on #{mod} is declared as `abstract`. It does not have an implementation.")
        end
      # Note, this logic is duplicated (intentionally, for micro-perf) at `CallValidation.wrap_method_if_needed`,
      # make sure to keep changes in sync.
      elsif method_sig.check_level == :always || (method_sig.check_level == :tests && T::Private::RuntimeLevels.check_tests?)
        CallValidation.validate_call(self, original_method, method_sig, args, blk)
      else
        original_method.bind(self).call(*args, &blk)
      end
    end

    new_method = mod.instance_method(method_name)
    key = method_to_key(new_method)
    @sig_wrappers[key] = sig_block
    if current_declaration.final
      add_final_method(key)
      # use hook_mod, not mod, because for example, we want class C to be marked as having final if we def C.foo as
      # final. change this to mod to see some final_method tests fail.
      add_module_with_final(hook_mod)
    end
  end

  # Executes the `sig` block, and converts the resulting Declaration
  # to a Signature.
  def self.run_sig(hook_mod, method_name, original_method, declaration_block)
    current_declaration =
      begin
        run_builder(declaration_block)
      rescue DeclBuilder::BuilderError => e
        T::Configuration.sig_builder_error_handler(e, declaration_block.loc)
        nil
      end

    signature =
      if current_declaration
        build_sig(hook_mod, method_name, original_method, current_declaration, declaration_block.loc)
      else
        Signature.new_untyped(method: original_method)
      end

    unwrap_method(hook_mod, signature, original_method)
    signature
  end

  def self.run_builder(declaration_block)
    builder = DeclBuilder.new(declaration_block.mod)
    builder
      .instance_exec(&declaration_block.blk)
      .finalize!
      .decl
  end

  def self.build_sig(hook_mod, method_name, original_method, current_declaration, loc)
    begin
      # We allow `sig` in the current module's context (normal case) and inside `class << self`
      if hook_mod != current_declaration.mod && hook_mod.singleton_class != current_declaration.mod
        raise "A method (#{method_name}) is being added on a different class/module (#{hook_mod}) than the " \
              "last call to `sig` (#{current_declaration.mod}). Make sure each call " \
              "to `sig` is immediately followed by a method definition on the same " \
              "class/module."
      end

      signature = Signature.new(
        method: original_method,
        method_name: method_name,
        raw_arg_types: current_declaration.params,
        raw_return_type: current_declaration.returns,
        bind: current_declaration.bind,
        mode: current_declaration.mode,
        check_level: current_declaration.checked,
        on_failure: current_declaration.on_failure,
        override_allow_incompatible: current_declaration.override_allow_incompatible,
      )

      SignatureValidation.validate(signature)
      signature
    rescue => e
      super_method = original_method&.super_method
      super_signature = signature_for_method(super_method) if super_method

      T::Configuration.sig_validation_error_handler(
        e,
        method: original_method,
        declaration: current_declaration,
        signature: signature,
        super_signature: super_signature
      )

      Signature.new_untyped(method: original_method)
    end
  end

  def self.unwrap_method(hook_mod, signature, original_method)
    maybe_wrapped_method = CallValidation.wrap_method_if_needed(signature.method.owner, signature, original_method)
    @signatures_by_method[method_to_key(maybe_wrapped_method)] = signature
  end

  def self.has_sig_block_for_method(method)
    has_sig_block_for_key(method_to_key(method))
  end

  private_class_method def self.has_sig_block_for_key(key)
    @sig_wrappers.key?(key)
  end

  def self.maybe_run_sig_block_for_method(method)
    maybe_run_sig_block_for_key(method_to_key(method))
  end

  private_class_method def self.maybe_run_sig_block_for_key(key)
    run_sig_block_for_key(key) if has_sig_block_for_key(key)
  end

  def self.run_sig_block_for_method(method)
    run_sig_block_for_key(method_to_key(method))
  end

  private_class_method def self.run_sig_block_for_key(key)
    blk = @sig_wrappers[key]
    if !blk
      sig = @signatures_by_method[key]
      if sig
        # We already ran the sig block, perhaps in another thread.
        return sig
      else
        raise "No `sig` wrapper for #{key_to_method(key)}"
      end
    end

    begin
      sig = blk.call
    rescue
      @sigs_that_raised[key] = true
      raise
    end
    if @sigs_that_raised[key]
      raise "A previous invocation of #{key_to_method(key)} raised, and the current one succeeded. Please don't do that."
    end

    @sig_wrappers.delete(key)
    sig
  end

  def self.run_all_sig_blocks
    loop do
      break if @sig_wrappers.empty?
      key, _ = @sig_wrappers.first
      run_sig_block_for_key(key)
    end
  end

  # the module target is adding the methods from the module source to itself. we need to check that for all instance
  # methods M on source, M is not defined on any of target's ancestors.
  def self._hook_impl(target, target_ancestors, source)
    if !module_with_final?(target) && !module_with_final?(source)
      return
    end
    add_module_with_final(target)
    install_hooks(target)
    _check_final_ancestors(target, target_ancestors - source.ancestors, source.instance_methods)
  end

  def self.set_final_checks_on_hooks(enable)
    is_enabled = @old_hooks != nil
    if enable == is_enabled
      return
    end
    if is_enabled
      @old_hooks.each(&:restore)
      @old_hooks = nil
    else
      old_included = T::Private::ClassUtils.replace_method(Module, :included) do |arg|
        old_included.bind(self).call(arg)
        ::T::Private::Methods._hook_impl(arg, arg.ancestors, self)
      end
      old_extended = T::Private::ClassUtils.replace_method(Module, :extended) do |arg|
        old_extended.bind(self).call(arg)
        ::T::Private::Methods._hook_impl(arg, arg.singleton_class.ancestors, self)
      end
      old_inherited = T::Private::ClassUtils.replace_method(Class, :inherited) do |arg|
        old_inherited.bind(self).call(arg)
        ::T::Private::Methods._hook_impl(arg, arg.ancestors, self)
      end
      @old_hooks = [old_included, old_extended, old_inherited]
    end
  end

  module MethodHooks
    def method_added(name)
      super(name)
      ::T::Private::Methods._on_method_added(self, name, is_singleton_method: false)
    end
  end

  module SingletonMethodHooks
    def singleton_method_added(name)
      super(name)
      ::T::Private::Methods._on_method_added(self, name, is_singleton_method: true)
    end
  end

  def self.install_hooks(mod)
    return if @installed_hooks.include?(mod)
    @installed_hooks << mod

    if mod.singleton_class?
      mod.include(SingletonMethodHooks)
    else
      mod.extend(MethodHooks)
    end
    mod.extend(SingletonMethodHooks)
  end

  # use this directly if you don't want/need to box up the method into an object to pass to method_to_key.
  private_class_method def self.method_owner_and_name_to_key(owner, name)
    "#{owner.object_id}##{name}"
  end

  private_class_method def self.method_to_key(method)
    method_owner_and_name_to_key(method.owner, method.name)
  end

  private_class_method def self.key_to_method(key)
    id, name = key.split("#")
    obj = ObjectSpace._id2ref(id.to_i) # rubocop:disable PrisonGuard/NoDynamicConstAccess
    obj.instance_method(name)
  end
end
