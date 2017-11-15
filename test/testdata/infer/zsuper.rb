class Foo
  def baz(a)
    puts a # error: does not exist
  end
end

class Bar < Foo
  def baz(b)
     super do |a|
       puts a # error: does not exist
     end
  end
end
