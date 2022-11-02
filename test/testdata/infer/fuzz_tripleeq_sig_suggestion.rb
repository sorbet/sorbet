# typed: strict

# This used to crash Sorbet because of a bug in SigSuggestion.

def foo(x) # error: The method `foo` does not have a `sig`
  Integer === x
end
