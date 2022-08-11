# typed: true

class Foo
  struct # error: does not exist
  #     ^ apply-completion: [A] item: 0
end

class Bar
  def foo
    struct = "xyz"
    struc # error: does not exist
    #  ^ completion: struct, ...
  end
end

class Baz
  def struct
    return 1
  end

  def bat
    struc # error: does not exist
    #    ^ completion: struct, ...
  end
end
