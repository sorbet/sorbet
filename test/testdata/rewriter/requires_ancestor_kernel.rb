# typed: true
# enable-experimental-requires-ancestor: true

# Test case 1: Module without any requires_ancestor should get Kernel added
module TestModule1
  def some_method
    puts "hello"
  end
end

# Test case 2: Module with requires_ancestor { BasicObject } should NOT get Kernel added
module TestModule2
  extend T::Helpers
  requires_ancestor { BasicObject }

  def some_method
    puts "hello" # error: Method `puts` does not exist on `TestModule2`
  end
end

# Test case 3: Module with requires_ancestor { Kernel } should NOT get another Kernel added
module TestModule3
  extend T::Helpers
  requires_ancestor { Kernel }

  def some_method
    puts "hello"
  end
end

# Test case 4: Class should NOT get requires_ancestor { Kernel } added
class TestClass1
  def some_method
    puts "hello"
  end
end

# Test case 5: Module with other requires_ancestor should get Kernel added
module TestModule4
  extend T::Helpers
  requires_ancestor { Object }

  def some_method
    puts "hello"
  end
end
