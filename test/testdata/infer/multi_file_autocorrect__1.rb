# typed: strict

class MyClass1
  def missing_sig; end # error: The method `missing_sig` does not have a `sig`
end
