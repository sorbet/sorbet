# typed: strict
class Module; include T::Sig; end

class AbstractBox
  extend T::Generic
  abstract!

  Elem = type_member
# ^ hierarchy-ref: Elem

  sig { abstract.returns(Elem) }
  #                      ^ hierarchy-ref: Elem
  def val; end
end

class ZeroBox < AbstractBox
  Elem = type_member { {fixed: Integer} }
# ^ hierarchy-ref: Elem

  sig { override.returns(Elem) }
  #                      ^ hierarchy-ref: Elem
  def val; 0; end
end

class GenericBox < AbstractBox
  Elem = type_member
# ^ hierarchy-ref: Elem

  sig { params(val: Elem).void }
  #                 ^ hierarchy-ref: Elem
  def initialize(val:)
    @val = val
  end

  sig { override.returns(Elem) }
  #                      ^ hierarchy-ref: Elem
  def val = @val
end
