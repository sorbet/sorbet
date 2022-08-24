# typed: true
module A
  extend T::Sig

  class C1; end

  sig {params(x: C1).returns(NilClass)}
  def f(x)
    C2 = type_template # error: dynamic constant assignment
    #    ^^^^^^^^^^^^^ error: Method `type_template` does not exist on `A`
    nil
  end
end
