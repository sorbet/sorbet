# typed: false
# disable-fast-path: true
class A
  sig {void.final}
  def foo
  end

  def bar
  end
end

module B
  sig {void.final}
  def bar
  end  
end

class C < A
  include B
  def foo # error: Method overrides a final method
  end

  def bar # error: Method overrides a final method
  end
end
