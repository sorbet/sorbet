# typed: true

# Regression Test for a UAF; Each `B` below generates an item on the
# resolver's TODO list with a raw pointer into the AST; Make sure that
# we don't delete the underlying AST when processing the assignment.
#
# Replicate this a few times just to make the bug more likely to
# manifest; Sanitized mode should catch it with one.
A = B # error: Unable to resolve constant
C = B # error: Unable to resolve constant
D = B # error: Unable to resolve constant
E = B # error: Unable to resolve constant
F = B # error: Unable to resolve constant
