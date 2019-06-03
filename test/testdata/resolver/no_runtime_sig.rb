# typed: strict

class Foo
  T::Sig::WithoutRuntime.sig {returns(Integer)}
  def bar
    91
  end
end

T.reveal_type(Foo.new.bar) # error: Integer
