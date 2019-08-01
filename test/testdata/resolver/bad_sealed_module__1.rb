# typed: false
# disable-fast-path: true

module Parent
  extend T::Helpers

  sealed!
end
class ChildGood1; include Parent; end
class ChildGood2; include Parent; end
