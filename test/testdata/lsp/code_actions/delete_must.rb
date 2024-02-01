# typed: true
# selective-apply-code-action: refactor.rewrite

xs = T::Array[Integer].new.first
T.must(xs).even?
# | apply-code-action: [A] Delete T.must
