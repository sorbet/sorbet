# typed: true
class Foo
  enum # error: does not exist
  #   ^ apply-completion: [A] item: 0
end

class Bar
  def foo
    enum = "xyz"
    enu # error: does not exist
    #  ^ completion: enum, ...
  end
end

class Baz
  def enum
    return 1
  end

  def bat
    enu # error: does not exist
    #  ^ completion: enum, ...
  end
end

class A
  def bar
    b = Baz.new
    b.enu # error: does not exist
    #    ^ completion: enum, ...
  end
end

class B
  def self.enum
    return "enum"
  end

  def self.bar
    self.enu # error: does not exist
    #       ^ completion: enum, ...
  end
end
