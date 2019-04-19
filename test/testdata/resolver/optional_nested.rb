# typed: true
# disable-fast-path: true
module M
  class C
    extend T::Sig

    sig {params(x: String).returns(String)}
    def self.id(x)
      x
    end
  end
end

class Test
  extend T::Sig

  sig {params(x: String).returns(String)}
  def foo(x = M::C.id(''))
    'hello, ' + x
  end

  sig {params(x: String).returns(String)}
  def bar(x = M::C.id(nil)) # error: `NilClass` doesn't match `String`
    'hello, ' + x
  end

  sig {params(x: Integer, y: String).returns(NilClass)}
  def qux(x, y: M::C.id(x)) # error: `Integer` doesn't match `String`
    puts 'hello, ' + y
  end
end
