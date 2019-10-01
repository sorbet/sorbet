# typed: true

class A
  extend T::Sig

  sig {returns(Integer)}
  private def my_method_private
    0
  end

  sig {returns(Integer)}
  def my_method_public
    0
  end

  sig {returns(Integer)}
  def foo
    my_method_ # error: does not exist
    #         ^ completion: my_method_private, my_method_public
  end
end

A.new.my_method_ # error: does not exist
#               ^ completion: my_method_public
