# typed: true

# Note: findSignature doesn't handle overloaded methods all that well right now

class I; end
class S; end
class A
  extend T::Sig
  sig {params(x: I).void}
  sig {params(x: S).void}
  def my_method(x) # error: against an overloaded signature
    #           ^ apply-rename: [A] newName: target placeholderText: x
    puts(x)
  end
end
