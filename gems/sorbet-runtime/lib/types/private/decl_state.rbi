# typed: strict

class T::Private::DeclState
  sig {returns(T::Private::DeclState)}
  def self.current; end

  sig {params(other: T::Private::DeclState).returns(T::Private::DeclState)}
  def self.current=(other); end

  sig { void }
  def initialize
    @active_declaration = T.let(nil, T.nilable(T::Private::Methods::DeclarationBlock))
    @skip_on_method_added = T.let(nil, T.nilable(TrueClass))
    @previous_declaration = T.let(nil, T.nilable(T::Private::Methods::DeclarationBlock))
  end

  sig {returns(T.nilable(T::Private::Methods::DeclarationBlock))}
  def active_declaration; end

  sig do
    params(active_declaration: T.nilable(T::Private::Methods::DeclarationBlock))
      .returns(T.nilable(T::Private::Methods::DeclarationBlock))
  end
  def active_declaration=(active_declaration); end

  sig { returns(T.nilable(T::Private::Methods::DeclarationBlock)) }
  def previous_declaration; end

  sig {returns(T.nilable(TrueClass))}
  def skip_on_method_added; end

  sig {params(skip_on_method_added: T.nilable(TrueClass)).returns(T.nilable(TrueClass))}
  def skip_on_method_added=(skip_on_method_added); end

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
