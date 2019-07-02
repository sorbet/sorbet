# frozen_string_literal: true
# typed: false

module T::Private::Methods
  @installed_hooks = Set.new
  @signatures_by_method = {}
  @sig_wrappers = {}
  @sigs_that_raised = {}

  ARG_NOT_PROVIDED = Object.new
  PROC_TYPE = Object.new

  DeclarationBlock = Struct.new(:mod, :loc, :blk)

  def self.declare_sig(mod, &blk)
    install_hooks(mod)

    if T::Private::DeclState.current.active_declaration
      T::Private::DeclState.current.reset!
      raise "You called sig twice without declaring a method inbetween"
    end

    loc = caller_locations(2, 1).first

    T::Private::DeclState.current.active_declaration = DeclarationBlock.new(mod, loc, blk)

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

  # Only public because it needs to get called below inside the replace_method blocks below.
  def self._on_method_added(hook_mod, method_name, is_singleton_method: false)
    current_declaration = T::Private::DeclState.current.active_declaration
    return if current_declaration.nil?
    T::Private::DeclState.current.reset!

    mod = is_singleton_method ? hook_mod.singleton_class : hook_mod
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
    @sig_wrappers[method_to_key(new_method)] = sig_block
  end

  def self.sig_error(loc, message)
    raise(ArgumentError.new("#{loc.path}:#{loc.lineno}: Error interpreting `sig`:\n  #{message}\n\n"))
  end

  # Executes the `sig` block, and converts the resulting Declaration
  # to a Signature.
  def self.run_sig(hook_mod, method_name, original_method, declaration_block)
    current_declaration =
      begin
        run_builder(declaration_block)
      rescue DeclBuilder::BuilderError => e
        T::Private::ErrorHandler.handle_sig_builder_error(e, declaration_block.loc)
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

      if current_declaration.returns.equal?(ARG_NOT_PROVIDED)
        sig_error(loc, "You must provide a return type; use the `.returns` or `.void` builder methods. Method: #{original_method}")
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
        generated: current_declaration.generated,
      )

      SignatureValidation.validate(signature)
      signature
    rescue => e
      super_method = original_method&.super_method
      super_signature = signature_for_method(super_method) if super_method

      T::Private::ErrorHandler.handle_sig_validation_error(
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
      raise "No `sig` wrapper for #{key_to_method(key)}"
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

  def self.install_hooks(mod)
    return if @installed_hooks.include?(mod)
    @installed_hooks << mod

    if mod.singleton_class?
      install_singleton_method_added_hook(mod)
    else
      original_method = T::Private::ClassUtils.replace_method(mod.singleton_class, :method_added) do |name|
        T::Private::Methods._on_method_added(self, name)
        original_method.bind(self).call(name)
      end
    end
    install_singleton_method_added_hook(mod.singleton_class)
  end

  private_class_method def self.install_singleton_method_added_hook(singleton_klass)
    attached = nil
    original_singleton_method = T::Private::ClassUtils.replace_method(singleton_klass, :singleton_method_added) do |name|
      attached = self
      T::Private::Methods._on_method_added(self, name, is_singleton_method: true)
      # This will be nil when this gets called for the addition of this method itself. We
      # call it below to compensate.
      if original_singleton_method
        original_singleton_method.bind(self).call(name)
      end
    end
    # See the comment above
    original_singleton_method.bind(attached).call(:singleton_method_added)
  end

  private_class_method def self.method_to_key(method)
    "#{method.owner.object_id}##{method.name}"
  end

  private_class_method def self.key_to_method(key)
    id, name = key.split("#")
    obj = ObjectSpace._id2ref(id.to_i) # rubocop:disable PrisonGuard/NoDynamicConstAccess
    obj.instance_method(name)
  end
end
