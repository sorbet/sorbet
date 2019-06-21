# typed: strict

# This used to crash Sorbet because of a bug in SigSuggestion.

def foo(x)
  Integer === x
end
