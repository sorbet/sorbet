# typed: true

module T::Utils
  # Only one caller--move into abstract validate, instead of typing externally
  sig { params(mod: Module).returns(T::Array[Symbol]) }
  def self.methods_excluding_object(mod)
  end

  # Should be in rbi/sorbet/t.rbi, but should this accept arbitrary type
  # syntax? Unclear, leaving the type private for now.
  sig { params(type: T.any(Module, T::Types::Base)).returns(T::Types::Base) }
  def self.resolve_alias(type); end

  # Should be in rbi/sorbet/t.rbi, but that would require exposing
  # T::Private::Methods::Signature, which is bigger than I want to tackle rn
  sig { params(method: UnboundMethod).returns(T.nilable(T::Private::Methods::Signature)) }
  def self.signature_for_method(method); end

  # Should be in rbi/sorbet/t.rbi, but that would require exposing
  # T::Private::Methods::Signature, which is bigger than I want to tackle rn
  sig do
    params(
      mod: T::Module[T.anything],
      method_name: Symbol
    )
      .returns(T.nilable(T::Private::Methods::Signature))
  end
  def self.signature_for_instance_method(mod, method_name); end

  sig do
    params(
      mod: T::Module[T.anything],
      method_sig: T::Private::Methods::Signature,
      original_method: UnboundMethod
    )
      .returns(UnboundMethod)
  end
  def self.wrap_method_with_call_validation_if_needed(mod, method_sig, original_method); end

  # TODO(jez) If we were to move this to the public rbi/ folder, we would
  # probably want the types to all be non-nil. Maybe we should just make this a
  # private helper.
  sig do
    type_parameters(:U)
      .params(
        str: T.any(T.all(T.type_parameter(:U), NilClass), String),
        start_len: T.nilable(Integer),
        end_len: T.nilable(Integer),
        ellipsis: String
      )
      .returns(T.any(T.all(T.type_parameter(:U), NilClass), String))
  end
  def self.string_truncate_middle(str, start_len, end_len, ellipsis='...'); end
end
