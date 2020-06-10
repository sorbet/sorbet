# frozen_string_literal: true
# typed: true
# compiled: true

module A
  private_class_method def self.pcm_that_yields(&blk)
    yield
  end

  def self.main(&blk)
    pcm_that_yields(&blk)
  end
end

res = A.main do
  255
end
p res
