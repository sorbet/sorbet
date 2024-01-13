# typed: true

# Prevents regression against an off-by-one error we used to have that would
# stop before suggestion similar constants that existed on <root>

class A
  extend T::Sig
  sig {returns(Int)} # error: Unable to resolve
  #               ^ completion: Integer, Interrupt
  def foo
    0
  end
end
