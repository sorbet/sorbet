# typed: true
class Foo
  def baz(a)
    puts a
  end
end

class Bar < Foo
  def baz(b)
     super do |a|
       puts a
     end
  end
end
