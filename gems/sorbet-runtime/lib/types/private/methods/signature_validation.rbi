# typed: true

module T::Private::Methods
  module SignatureValidation
    sig {params(signature: Signature).void}
    def self.validate(signature); end

    sig {params(mode: String).returns(String)}
    private_class_method def self.pretty_mode(mode); end

    sig {params(signature: Signature, super_signature: Signature).void}
    def self.validate_override_mode(signature, super_signature); end

    sig do
      params(
        mode: String,
        method_name: Symbol,
        method: UnboundMethod,
        source_loc: T.nilable([String, Integer]),
      )
        .void
    end
    def self.validate_non_override_mode(mode, method_name, method, source_loc = nil); end

    sig {params(signature: Signature, super_signature: Signature).void}
    def self.validate_override_shape(signature, super_signature); end

    sig {params(signature: Signature, super_signature: Signature).void}
    def self.validate_override_types(signature, super_signature); end

    sig {params(signature: Signature, super_signature: Signature).void}
    def self.validate_override_visibility(signature, super_signature); end

    sig {params(method: UnboundMethod).returns(Symbol)}
    private_class_method def self.method_visibility(method); end

    sig {params(vis: Symbol).returns(Integer)}
    private_class_method def self.visibility_strength(vis); end

    sig {params(signature: Signature, super_signature: Signature).void}
    private_class_method def self.base_override_loc_str(signature, super_signature); end
  end
end
