# typed: true
alias foo foo
def foo; end # error: Too many alias expansions for symbol ::Object#foo, the alias is either too long or infinite.

