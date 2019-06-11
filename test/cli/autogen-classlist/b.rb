# typed: true

class Foo
  def my_method
    class InMethod; end # Not allowed
  end
end
