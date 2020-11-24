# typed: false

class ChildBad3
  include Parent # error: `Parent` is sealed and cannot be included in `ChildBad3`
end
class ChildBad4; include Parent; end # error: `Parent` is sealed and cannot be included in `ChildBad4`
