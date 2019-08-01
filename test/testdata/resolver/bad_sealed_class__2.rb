# typed: false

class ChildBad3 < AbstractParent # error: `AbstractParent` is sealed and cannot be inherited by `ChildBad3`
end
class ChildBad4 < AbstractParent; end # error: `AbstractParent` is sealed and cannot be inherited by `ChildBad4`
