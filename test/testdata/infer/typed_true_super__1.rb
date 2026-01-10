# typed: true
class Module; include T::Sig; end

class Child1 < Parent
  # No sig is fine--the Ruby VM will forward the
  # block arg, if passed a block
  def self.foo
    x = super
    T.reveal_type(x) # error: `Integer`
  end
end

Child1.foo do
  # this *does* get called
  puts 'hello'
end

class Child2 < Parent
  sig {returns(Integer)}
  def self.foo
    x = super
    #   ^^^^^ error: Expected `T.proc.void` but found `NilClass` for block argument
    T.reveal_type(x) # error: `Integer`
  end
end

Child2.foo do # error: does not take a block
  # this *does* get called
  puts 'hello'
end

class Child3 < Parent
  sig {params(blk: T.proc.params(x: Integer).void).returns(Integer)}
  def self.foo(&blk)
    x = super
    #   ^^^^^ error: Expected `T.proc.void` but found `T.proc.params(arg0: Integer).void` for block argument
    T.reveal_type(x) # error: `Integer`
  end
end

Child3.foo do
  # this *does* get called
  puts 'hello'
end
