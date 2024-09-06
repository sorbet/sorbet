# typed: true

require_relative './child'

module Bopus
  ChildAlias = Child
  class ChildOfChild1 < Child; end
  class ChildOfChild2 < ChildAlias; end
end
