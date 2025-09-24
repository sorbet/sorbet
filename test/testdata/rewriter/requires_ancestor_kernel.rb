# typed: true
# enable-experimental-requires-ancestor: true

# Module without any `requires_ancestor` should get `Kernel` added
module TestModule1
  def some_method
    puts "hello"
  end
end

# Module with `requires_ancestor { BasicObject }` should NOT get `Kernel` added
module TestModule2
  extend T::Helpers
  requires_ancestor { BasicObject }

  def some_method
    puts "hello" # error: Method `puts` does not exist on `TestModule2`
  end
end

# Module with `requires_ancestor { Kernel }` should NOT get another `Kernel` added
module TestModule3
  extend T::Helpers
  requires_ancestor { Kernel }

  def some_method
    puts "hello"
  end
end

# Class should NOT get `requires_ancestor { Kernel }` added
class TestClass1
  def some_method
    puts "hello"
  end
end

# Module with other `requires_ancestor` should get `Kernel` added
module TestModule4
  extend T::Helpers
  requires_ancestor { Object }

  def some_method
    puts "hello"
  end
end

# Unscoped `Kernel` modules should NOT get `Kernel` added
module Kernel
end

module ::Kernel
end

# Scoped modules named `Kernel` should get `Kernel` added
module Mine::Kernel
end

# Unfortunately, nested constants that are named Kernel will not get Kernel added
module Mine
  module Kernel
  end
end
