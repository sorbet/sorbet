# typed: true
# spacer for exclude-from-file-update

class Parent; end
class Child < Parent; end

T.let(Box[Child].new, Box[Parent]) # error: does not have asserted type
