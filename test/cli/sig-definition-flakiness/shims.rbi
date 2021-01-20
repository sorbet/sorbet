# typed: true

module Test
  sig { returns(Integer) } # conflicts with dsl.rbi
  def foo
  end
end

