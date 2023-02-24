# typed: true
# disable-fast-path: true

module Parent
  extend T::Generic

  initializable!
end

module ChildModule1 # error: `initializable!` declared by parent `Parent` must be re-declared in `ChildModule1
  include Parent
end

module ChildModule2 # error: `Parent` was declared `initializable!` and so cannot be `extend`ed into the module `ChildModule2
  extend Parent
end

class ChildClass # error: `Parent` was declared `initializable!` and so must be `extend`ed into the class `ChildClass
  include Parent
end
