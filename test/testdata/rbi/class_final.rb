# typed: strict

class BadClass < Class
  #              ^^^^^ error: `Class` was declared as `final!` and cannot be inherited by `BadClass`
end
