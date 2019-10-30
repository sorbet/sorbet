class A
  def foo
    1
  end
end
class B < A
end
puts B.new.foo
