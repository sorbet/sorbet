# typed: true
class Module; include T::Sig; end

class Child < Parent
  # No sig is fine--the Ruby VM will forward the
  # block arg, if passed a block
  def initialize
    # TODO: This error is over-eager.
    super # error: requires a block parameter
  end
end

Child.new do
  # This *will* get called.
  puts 'hello'
end
