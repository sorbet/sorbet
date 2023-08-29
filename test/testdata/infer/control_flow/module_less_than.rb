# typed: true
extend T::Sig

class A
  def self.foo(x)
    x.to_s
  end
end

sig {params(y: Object).void}
def main(y)
  if y.is_a?(Module) && y < A
    y.foo("bar") 
  end
end

