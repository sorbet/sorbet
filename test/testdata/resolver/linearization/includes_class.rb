# typed: true

class Sup
end
class A < Sup
end

class B # error: Only modules can be `include`d
  include A
end

module C # error: Only modules can be `include`d
  include A
  def bla
    raise "s"
  end
end

module IncludesBO
  include BasicObject # we permit this, because modules always will effectively include basicObject
end
