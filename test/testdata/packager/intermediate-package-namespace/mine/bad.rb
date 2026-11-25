# typed: strict

module Shared::Other # error: File belongs to package `Shared::Mine` but defines a constant that does not match this namespace
  class FromIntermediate
  end
end
