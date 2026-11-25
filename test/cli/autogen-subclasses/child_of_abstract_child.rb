# typed: true

require_relative './child'

module Bopus
  AbstractChildAlias = AbstractChild
  class ChildOfAbstractChild1 < AbstractChild
    abstract!
  end
  class ChildOfAbstractChild2 < AbstractChildAlias; end
end
