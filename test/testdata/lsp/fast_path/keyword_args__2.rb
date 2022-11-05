# typed: true
# spacer for exclude-from-file-update

module B
  extend T::Sig

  def some_other_method
    A.some_method("")
  end
end
