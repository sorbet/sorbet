# typed: true
class A
  extend T::Sig
  def foo(x)
    if x
    puts 'hello'
  end

  sig {void}
  def bar
  end
end # error: unexpected token "end of file"
