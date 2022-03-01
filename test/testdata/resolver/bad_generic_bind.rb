# typed: true

class TypeTemplate
  extend T::Sig
  extend T::Generic

  Elem = type_template

  sig { params(block: T.proc.bind(Elem).void).void }
                    # ^^^^^^^^^^^^^^^^^ error: Malformed `bind`: Can only bind to simple class names
  def self.before_create(&block); end
end

class TypeMember
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig { params(block: T.proc.bind(Elem).void).void }
                    # ^^^^^^^^^^^^^^^^^ error: Malformed `bind`: Can only bind to simple class names
  def before_create(&block); end
end
