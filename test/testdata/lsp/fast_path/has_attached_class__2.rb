# typed: true
# spacer for exclude-from-file-update
# Disables re-running a no-op file edit over just this file.
# The rbupdate fast path test will still happen
# disable-fast-path: true

class AbstractModel; end
class Parent < AbstractModel # error: Type variance mismatch
  include Inheritable
end

T.let(Parent, Inheritable::ClassMethods[AbstractModel]) # error: does not have asserted type
