# typed: true

module M
  class C
    extend T::Helpers

    sig(x: String).returns(String)
    def self.id(x)
      x
    end
  end
end

class Test
  extend T::Helpers

  sig(x: String).returns(String)
  def foo(x = M::C.id(''))
    'hello, ' + x
  end

  sig(x: String).returns(String)
  def bar(x = M::C.id(nil)) # error: `NilClass` doesn't match `String`
    'hello, ' + x
  end

  sig(x: Integer, y: String).returns(NilClass)
  def qux(x, y: M::C.id(x)) # error: `Integer` doesn't match `String`
    puts 'hello, ' + y
  end
end
