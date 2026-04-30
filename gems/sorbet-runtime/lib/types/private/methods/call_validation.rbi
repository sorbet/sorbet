# typed: true

module T::Private::Methods
  module CallValidation
    sig do
      params(
        mod: Module,
        method_sig: Signature,
        original_method: UnboundMethod
      )
        .returns(UnboundMethod)
    end
    def self.wrap_method_if_needed(mod, method_sig, original_method); end

    @is_allowed_to_have_fast_path = T.let(true, T::Boolean)

    sig { returns(T::Boolean) }
    def self.is_allowed_to_have_fast_path; end

    sig { void }
    def self.disable_fast_path; end

    sig do
      params(
        mod: Module,
        method_sig: Signature,
        original_method: UnboundMethod,
        original_visibility: Symbol
      )
        .void
    end
    def self.create_abstract_wrapper(mod, method_sig, original_method, original_visibility); end

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
