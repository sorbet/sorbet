# typed: true
module A
  extend T::Sig

  class C1; end

  sig {params(x: C1).returns(NilClass)}
  def f(x)
    C2 = type_member # parser-error: dynamic constant assignment
    #    ^^^^^^^^^^^ error: Method `type_member` does not exist on `A`
    nil
  end
end
