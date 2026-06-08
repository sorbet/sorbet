# typed: strict

class T::Private::DeclState
  sig {returns(T::Private::DeclState)}
  def self.current; end

  sig {params(other: T::Private::DeclState).returns(T::Private::DeclState)}
  def self.current=(other); end

  sig {returns(T.nilable(T::Private::Methods::DeclarationBlock))}
  attr_accessor :active_declaration

  sig {returns(T.nilable(TrueClass))}
  attr_accessor :skip_on_method_added

  sig { returns(T.nilable(T::Private::Methods::DeclarationBlock)) }
  attr_accessor :previous_declaration

  sig {void}
  def reset!; end

  sig { returns(T.nilable(T::Private::Methods::DeclarationBlock)) }
  def consume!; end

  sig do
    type_parameters(:U)
      .params(blk: T.proc.returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  end
  def without_on_method_added(&blk); end
end
