# typed: true

module Test
  sig { returns(String) } # conflicts with shims.rbi
  def foo
  end
end

