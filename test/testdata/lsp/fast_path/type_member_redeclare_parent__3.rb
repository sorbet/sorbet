# typed: strict
# spacer for exclude-from-file-update

class GrandChild < Child # error: Type `MyElem` declared by parent `Child` must be re-declared in `GrandChild`
end
