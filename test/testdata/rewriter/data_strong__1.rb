# typed: strong

Point = Data.define(:x, :y) do
  extend T::Sig

  sig { params(x: Integer, y: Integer).void }
  def initialize(x:, y:)
    super
  end
end
