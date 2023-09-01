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
  # This one is a little weird. Sorbet allows this method to be called with a
  # block, even though the sig doesn't take a block, because this method is
  # defined in a `# typed: true` file.
  sig {returns(Integer)}
  def self.foo
    x = super
    T.reveal_type(x) # error: `Integer`
  end
end

Child2.foo do
  # this *does* get called
  puts 'hello'
end
