# @typed
class Foo;
  def self.bar
  end; 
  def baz
  end
end; 

Foo.bar
Foo.baz # error: does not exist on
