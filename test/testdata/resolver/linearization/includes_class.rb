# typed: true

class Sup
end
class A < Sup
end

class B
  include A # error: Only modules can be `include`d, but `A` is a class
  extend A # error: Only modules can be `extend`d, but `A` is a class
end

module C
  include A # error: Only modules can be `include`d, but `A` is a class
  extend A # error: Only modules can be `extend`d, but `A` is a class
  def bla
    raise "s"
  end
end

module IncludesBO
  include BasicObject # we permit this, because modules always will effectively include basicObject
end
