# typed: true
# spacer for assert-fast-path
# spacer for exclude-from-file-update

class A
  extend T::Sig
  sig {returns(Integer)}
  def to_method
    0
  end
end
