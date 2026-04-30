# typed: true

module T::Private::Methods
  class DeclarationBlock
    sig {returns(Module)}
    attr_accessor :mod

    sig {returns(T.nilable(Thread::Backtrace::Location))}
    attr_accessor :loc

    sig {returns(T.nilable(T.any(T.proc.returns(DeclBuilder), Declaration)))}
    attr_accessor :blk_or_decl

    sig {returns(T::Boolean)}
    attr_accessor :final

    sig {returns(T::Boolean)}
    attr_accessor :raw

    sig do
      params(
        mod: Module,
        loc: T.nilable(Thread::Backtrace::Location),
        blk: Proc,
        final: T::Boolean,
        raw: T::Boolean
      )
        .void
    end
    def initialize(mod, loc, blk, final, raw); end
  end

  @installed_hooks = T.let({}, T::Hash[Module, TrueClass])
  @modules_with_final = T.let({}, T::Hash[Module, T::Hash[Symbol, TrueClass]])
  @was_ever_final_names = T.let({}, T::Hash[Symbol, TrueClass])
  @sig_wrappers = T.let({}, T::Hash[String, T.proc.returns(Signature)])
  @signatures_by_method = T.let({}, T::Hash[String, Signature])
  @sigs_that_raised = T.let({}, T::Hash[String, TrueClass])
  @old_hooks = T.let(nil, T.nilable([UnboundMethod, UnboundMethod, UnboundMethod]))

  sig {params(mod: Module, loc: T.nilable(Thread::Backtrace::Location), arg: T.nilable(Symbol), blk: T.untyped).returns(NilClass)}
  def self.declare_sig(mod, loc, arg, &blk); end

  sig {params(mod: Module, arg: T.nilable(Symbol), blk: T.untyped).returns(DeclarationBlock)}
  def self._declare_sig(mod, arg=nil, &blk); end

  sig {params(mod: Module, declblock: T.nilable(DeclarationBlock), blk: T.untyped).void}
  def self._with_declared_signature(mod, declblock, &blk); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def self.start_proc; end

  sig {params(decl: T::Private::Methods::Declaration).returns(T::Types::Proc)}
  def self.finalize_proc(decl); end

  SORBET_RUNTIME_LIB_PATH = T.let("", String)

  IS_TYPECHECKING = T.let(false, T::Boolean)

  sig {params(target: Module, source_method_names: T::Array[Symbol], source: T.nilable(Module)).void}
  def self._check_final_ancestors(target, source_method_names, source); end

  sig {params(mod: Module, method_name: Symbol).returns(NilClass)}
  def self.add_module_with_final_method(mod, method_name); end

  sig {params(mod: Module).void}
  def self.note_module_deals_with_final(mod); end

  sig {params(hook_mod: Module, mod: Module, method_name: Symbol).void}
  def self._on_method_added(hook_mod, mod, method_name); end

  sig {params(receiver: Object, original_method: UnboundMethod, callee: Symbol).returns(T::Private::Methods::Signature)}
  def self._handle_missing_method_signature(receiver, original_method, callee); end

  sig {params(hook_mod: Module, method_name: Symbol, original_method: UnboundMethod, declaration_block: DeclarationBlock).returns(T::Private::Methods::Signature)}
  def self.run_sig(hook_mod, method_name, original_method, declaration_block); end

  sig {params(declaration_block: DeclarationBlock).returns(T::Private::Methods::Declaration)}
  def self.run_builder(declaration_block); end

  sig {params(hook_mod: Module, method_name: Symbol, original_method: UnboundMethod, current_declaration: T::Private::Methods::Declaration).returns(T::Private::Methods::Signature)}
  def self.build_sig(hook_mod, method_name, original_method, current_declaration); end

  sig {params(method: T.any(Method, UnboundMethod)).returns(T.nilable(T::Private::Methods::Signature))}
  def self.signature_for_method(method); end

  sig {params(mod: Module, signature: T::Private::Methods::Signature, original_method: UnboundMethod).void}
  def self.unwrap_method(mod, signature, original_method); end

  sig {params(method: UnboundMethod).returns(T::Boolean)}
  def self.has_sig_block_for_method(method); end

  sig {params(method: UnboundMethod).returns(T.nilable(T::Private::Methods::Signature))}
  def self.maybe_run_sig_block_for_method(method); end

  sig {params(key: String).returns(T.nilable(T::Private::Methods::Signature))}
  def self.maybe_run_sig_block_for_key(key); end

  sig {params(method: UnboundMethod).returns(T::Private::Methods::Signature)}
  def self.run_sig_block_for_method(method); end

  sig {params(force_type_init: T::Boolean).void}
  def self.run_all_sig_blocks(force_type_init: true); end

  sig {returns(T::Array[T::Private::Methods::Signature])}
  def self.all_checked_tests_sigs; end

  sig {params(target: Module, source: Module).void}
  def self._hook_impl(target, source); end

  sig {params(enable: T::Boolean).void}
  def self.set_final_checks_on_hooks(enable); end

  sig {params(mod: T::Module[T.anything]).void}
  def self.install_hooks(mod); end

  sig {params(mod: Module, loc: T.nilable(Thread::Backtrace::Location), arg: T.nilable(Symbol), raw: T::Boolean, blk: Proc).returns(DeclarationBlock)}
  private_class_method def self._declare_sig_internal(mod, loc, arg, raw: false, &blk); end

  sig {params(key: String).returns(T.nilable(T::Private::Methods::Signature))}
  private_class_method def self.signature_for_key(key); end

  sig {params(key: String).returns(T::Boolean)}
  private_class_method def self.has_sig_block_for_key(key); end

  sig {params(owner: Module, name: T.any(Symbol, String)).returns(String)}
  private_class_method def self.method_owner_and_name_to_key(owner, name); end

  sig {params(method: T.any(Method, UnboundMethod)).returns(String)}
  private_class_method def self.method_to_key(method); end

  sig {params(key: String, force_type_init: T::Boolean).returns(T::Private::Methods::Signature)}
  private_class_method def self.run_sig_block_for_key(key, force_type_init: false); end

  module MethodHooks
    sig {params(name: Symbol).void}
    def method_added(name); end
  end

  module SingletonMethodHooks
    sig {returns(Class)}
    def singleton_class; end

    sig {params(name: Symbol).void}
    def singleton_method_added(name); end
  end
end
