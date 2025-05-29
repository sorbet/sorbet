
# It's important that the name in the class decl be an existing module name
class Enumerable # error: `Enumerable` was previously defined as a `module`
  extend T::Generic
        # ^ completion: DATA, ...
  X = type_member
end
