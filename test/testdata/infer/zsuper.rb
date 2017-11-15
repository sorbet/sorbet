class Foo
  def baz(a)
    puts a # error: does not exist
  end
end

class Bar < Foo
  def baz(b)
     super
  end
end
