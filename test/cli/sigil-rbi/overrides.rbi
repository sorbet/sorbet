# typed: true

class S
  include Enumerable
  sig { params(blk: Proc).void }
  def each(&blk); end
end
