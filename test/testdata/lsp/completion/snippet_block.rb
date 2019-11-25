# typed: true

class Module
  include T::Sig
end

class A
  sig {params(blk: T.proc.void).void}
  def self.required_block(&blk)
    yield
  end

  sig {params(blk: T.nilable(T.proc.void)).void}
  def self.nilable_block(&blk)
    yield if block_given?
  end

  sig {params(blk: T.untyped).void}
  def self.untyped_block(&blk)
    yield if block_given?
  end

  def self.unsigged_block(&blk)
    yield if block_given?
  end
end

A.required_bloc # error: does not exist
#              ^ apply-completion: [A] item: 0
A.nilable_bloc # error: does not exist
#             ^ apply-completion: [B] item: 0
A.untyped_bloc # error: does not exist
#             ^ apply-completion: [C] item: 0
A.unsigged_bloc # error: does not exist
#              ^ apply-completion: [D] item: 0
