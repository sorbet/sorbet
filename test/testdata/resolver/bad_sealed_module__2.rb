# typed: false

class ChildBad3
  include Parent # error-with-dupes: `Parent` is sealed and cannot be included in `ChildBad3`
end
class ChildBad4; include Parent; end # error-with-dupes: `Parent` is sealed and cannot be included in `ChildBad4`
