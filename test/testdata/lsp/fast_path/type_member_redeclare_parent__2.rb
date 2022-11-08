# typed: strict
# spacer for exclude-from-file-update

class Child < Parent # error: Type `MyElem` declared by parent `Parent` must be re-declared in `Child`
end
