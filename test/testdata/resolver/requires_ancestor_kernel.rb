# typed: true
# enable-experimental-requires-ancestor: true

module MyHelper
  extend T::Helpers

  requires_ancestor { Kernel }

  def error(message)
    puts "An error occurred: #{message}"
    raise message
  end
end

class MyTest1
  include MyHelper

  def test
    error("test1")
  end
end

class MyTest2 < BasicObject # error: `MyTest2` must include `Kernel` (required by `MyHelper`)
  include MyHelper

  def test
    error("test2")
  end
end

module MyTest3
  include MyHelper
end

class MyTest4 < BasicObject
  extend T::Helpers
  include MyHelper

  abstract!
end

class MyTest5 < MyTest4 # error: `MyTest5` must include `Kernel` (required by `MyHelper`)
end
