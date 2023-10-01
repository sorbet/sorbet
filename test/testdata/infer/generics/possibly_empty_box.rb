# typed: strict
class Module; include T::Sig; end

# TODO(jez) All the 'Expression does not have a fully-defined type' errors are
# coming from calls to externalType on PossiblyEmptyBox, which happens any time
# there's a constant lit. We will have to change externalType to also make the
# external-ness apply to the bounds (it assumes all bounds are fully-defined
# right now and so returns them verbatim.)

class PossiblyEmptyBox
  extend T::Generic

  class IsEmpty; end
  IS_EMPTY = T.let(IsEmpty.new.freeze, IsEmpty)
  private_constant :IS_EMPTY

  Elem = type_member(:out)

  MaybeElem = type_member(:out) {
    {fixed: T.any(IsEmpty, Elem)}
  }

  sig { params(empty: T::Boolean, val: T.nilable(Elem)).void }
  def initialize(empty, val)
    @empty = empty
    @val = val
  end

  private_class_method :new

  sig do
    type_parameters(:Elem)
      .params(val: T.type_parameter(:Elem))
      .returns(T.all(T.attached_class, PossiblyEmptyBox[T.type_parameter(:Elem)]))
  end
  def self.make(val)
    if T.unsafe(nil)
      self.new(false, val)
    end

    self[T.type_parameter(:Elem)].new(false, val)
  end

  sig do
    type_parameters(:Elem)
      .returns(T.all(T.attached_class, PossiblyEmptyBox[T.noreturn]))
  end
  def self.empty
    if T.unsafe(nil)
      return self.new(true, nil)
    end

    self[T.noreturn].new(true, nil)
  end

  sig { returns(Elem) }
  def val!
    raise ArgumentError.new("Called .val!, but box was empty!") if @empty
    T.must(@val)
  end

  sig { returns(MaybeElem) }
  def val
    if @empty
      return IS_EMPTY
    else
      return T.must(@val)
    end
  end

  sig { returns(MaybeElem) }
  def val_broken
    if T.unsafe(nil)
      return val
    end

    if @empty
      return nil
    else
      return @val
    end
  end
end
