# typed: true

class Test
  extend T::Sig

  sig {params(x: Integer).void}
  def x_is_integer(x); end
end

Test.new.x_is_ # error: does not exist
#             ^ apply-completion: [A] item: 0
