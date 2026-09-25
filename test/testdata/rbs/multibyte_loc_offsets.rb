# typed: true
# enable-experimental-rbs-comments: true

# Regression test for multibyte character loc offsets, to catch any conflation of character count with byte count.
# * The emoji family (👨‍👩‍👧‍👦) is 7 characters but 25 bytes (4 people + 3 ZWJ).
# * The pizza is 1 character but 4 bytes.

#: ("👨‍👩‍👧‍👦", "🍕") -> void
#    ^^^^^^^^^^^^^^^^^^^^^^^^^ error: RBS literal types are not supported
#                               ^^^^ error: RBS literal types are not supported
def f(i, j) = i.to_s
