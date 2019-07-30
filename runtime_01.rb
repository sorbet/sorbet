class Parent
  def self.inherited(other)
    puts "(#{__FILE__}) inherited:"
    puts caller
    puts
  end
end

module A
  class Child1 < Parent; end
  Child2 = Class.new(Parent)
end

require_relative './runtime-02'
