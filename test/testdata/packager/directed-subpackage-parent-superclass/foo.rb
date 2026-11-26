# typed: true

module MyPackage
  class Base
    def from_base; end
  end

  class Foo < Base
  end
end

MyPackage::Foo.new.from_base
