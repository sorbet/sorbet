# typed: true

module A
  class Foo
    include ::X
  end

  class Bar
    some_method do
      include ::Y
    end
  end
end
