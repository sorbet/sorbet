# typed: true

module MyPackage
  class Base
    def from_base; end
  end

  class Foo < Base
  end

  Foo.new.from_base
end
