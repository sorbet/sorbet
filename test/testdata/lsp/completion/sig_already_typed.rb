# typed: true

class A
  extend T::Sig

  sig {void}
  #  ^ apply-completion: [A] item: 0
  def already_sigged
    42
  end
end
