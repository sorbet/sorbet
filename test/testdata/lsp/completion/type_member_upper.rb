# typed: true

class A
  def method_on_a; end
end

class BoxA
  extend T::Sig
  extend T::Generic

  Elem = type_member {{upper: A}}

  sig {params(x: Elem).void}
  def example(x)
    x.method_on_  # error: does not exist
    #           ^ completion: method_on_a
  end
end
