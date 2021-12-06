# typed: true
extend T::Sig

# Tests an edge case arising from the difference between <root> and Object

sig # error: no block
#  ^ apply-completion: [A] item: 0
def foo; end
