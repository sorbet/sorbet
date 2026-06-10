# typed: true

module T::Private::Methods
  module CallValidation
    sig do
      params(
        mod: Module,
        method_sig: Signature,
        original_method: UnboundMethod
      )
        .void
    end
    def self.wrap_method_if_needed(mod, method_sig, original_method); end

    # Defined via module_eval at load time in call_validation.rb (per-arity
    # unrolled validator families); declared here so they exist statically.
    # The create_validator_method_optional_args*_* family is dispatched via
    # send with a computed name and needs no declarations.
    def self.create_validator_method_kwargs0(mod, original_method, method_sig, original_visibility, return_type, kwarg_types); end
    def self.create_validator_method_with_block0(mod, original_method, method_sig, original_visibility, return_type, block_type); end
    def self.create_validator_method_with_block1(mod, original_method, method_sig, original_visibility, return_type, block_type, arg0_type); end
    def self.create_validator_method_with_block2(mod, original_method, method_sig, original_visibility, return_type, block_type, arg0_type, arg1_type); end
    def self.create_validator_method_with_block3(mod, original_method, method_sig, original_visibility, return_type, block_type, arg0_type, arg1_type, arg2_type); end
    def self.create_validator_method_with_block4(mod, original_method, method_sig, original_visibility, return_type, block_type, arg0_type, arg1_type, arg2_type, arg3_type); end

    @is_allowed_to_have_fast_path = T.let(true, T::Boolean)

    sig { returns(T::Boolean) }
    def self.is_allowed_to_have_fast_path; end

    sig { void }
    def self.disable_fast_path; end

    sig do
      params(
        mod: Module,
        method_name: Symbol,
        original_visibility: Symbol
      )
        .void
    end
    def self.create_abstract_wrapper(mod, method_name, original_visibility); end

    sig do
      params(
        mod: Module,
        original_method: UnboundMethod,
        method_sig: Signature,
        original_visibility: Symbol
      )
        .void
    end
    def self.create_validator_method(mod, original_method, method_sig, original_visibility); end

    sig do
      params(
        mod: Module,
        original_method: UnboundMethod,
        method_sig: Signature,
        original_visibility: Symbol
      )
        .void
    end
    def self.create_validator_slow_skip_block_type(mod, original_method, method_sig, original_visibility); end

    sig do
      params(
        instance: T.untyped,
        original_method: UnboundMethod,
        method_sig: Signature,
        args: T::Array[Kernel],
        blk: Proc
      )
        .void
    end
    def self.validate_call_skip_block_type(instance, original_method, method_sig, args, blk); end
  end
end
