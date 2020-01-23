# frozen_string_literal: true
# typed: true
# compiled: true

[[1, 2]].each do |el1, el2|
  # if a block/proc is given a single array as an arg and it expects more than a single arg, array is expanded to arguments
  T.let(el1, Integer) + T.let(el2, Integer)
end
