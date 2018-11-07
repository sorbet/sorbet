# typed: true

class A
  extend T::Helpers

  def self.foo
    yield
  end

  foo do
    sig {returns(Integer)}
    def bar
      3
    end
  end
end

T.reveal_type(A.bar) # error: Revealed type: `Integer`
