# frozen_string_literal: true
# typed: true
# compiled: true

class C
  Data = T.let(['a', 'b', 'c'], T::Array[String])

  def self.all(&blk)
    blk ||= proc {|c| c + "x"}
    Data.map(&blk)
  end
end

p C.all {|c| c + "y"}
p C.all

