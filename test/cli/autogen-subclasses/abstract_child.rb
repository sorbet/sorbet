# typed: true

require_relative './parent'

module Bopus
  class AbstractChild < Parent
    abstract!
  end
end
