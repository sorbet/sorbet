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
