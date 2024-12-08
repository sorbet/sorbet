# typed: true

module DSL; end

class A
  class << self
    include DSL
  end
end

class B
  extend DSL
end
