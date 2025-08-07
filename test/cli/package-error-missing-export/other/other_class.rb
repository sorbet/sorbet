# typed: strict

module Foo::Bar::OtherPackage
  class OtherClass
    puts("behavior")
  end

  class NotExported
    puts("behavior")
  end

  class Inner::AlsoNotExported
    puts("behavior")
  end
end
