# frozen_string_literal: true
# typed: true
# compiled: true

def no_block
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq
  p [1, 2, 3, 2, 3, 4, 1, 1, 5, 8, 6].uniq
end


no_block

def with_block
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq { |x| x.reverse }
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq { |x| x.length }
  p [1, 2, 3, 2, 3, 4, 1, 1, 5, 8, 6].uniq { |x| x%2 }
  p [1, 2, 3, 4].uniq { break "ope" }
end


with_block
