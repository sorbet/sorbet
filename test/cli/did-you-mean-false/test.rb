# typed: true
extend T::Sig

class A
  def my_method
  end
end

A.new.my_metho

sig {params(x: Integer).void}
def takes_integer(x)
end

sig {params(x: T.nilable(Integer)).void}
def takes_nilable_integer(x)
  takes_integer(x)
end
