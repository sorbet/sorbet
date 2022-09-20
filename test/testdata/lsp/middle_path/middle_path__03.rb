# typed: true
# spacer for exclude-from-file-update

class A_03
  extend T::Sig

  sig {returns(Integer)}
  def foo()
    42
  end
end
