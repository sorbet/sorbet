# typed: strict
class BlockTest
  def block_pass
    foo(1, 2, 3) do |x, y| # error: does not exist
      x
    end
  end
end
