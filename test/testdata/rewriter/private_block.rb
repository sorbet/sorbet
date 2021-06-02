# typed: true

class PrivateStruct
  Foo = Struct.new(:foo) do
    private
  end

  Foo.new
end
