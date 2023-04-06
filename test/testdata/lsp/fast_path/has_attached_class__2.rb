# typed: true
# spacer for exclude-from-file-update

class AbstractModel; end
class Parent < AbstractModel
  include Inheritable
end

T.let(Parent, Inheritable::ClassMethods[AbstractModel]) # error: does not have asserted type
