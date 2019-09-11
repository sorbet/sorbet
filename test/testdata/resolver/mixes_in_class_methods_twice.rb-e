# typed: true

# This tests that we don't regress in LSP / incremental resolve mode.
#
# In incremental resolve mode, even if a class has only one call to
# mixes_in_class_methods, Sorbet sees it twice (once from the first pass, once
# from the second, incrementalResolve, pass).
#
# If the second one we see is the same as the one we saw previously, this is
# actually fine. But if the class is different, that's an error.

module A
  module ClassMethods1; end
  module ClassMethods2; end
  mixes_in_class_methods(ClassMethods1)
  mixes_in_class_methods(ClassMethods2) # error: Redeclaring `mixes_in_class_methods` from module `A::ClassMethods1` to module `A::ClassMethods2`
end

module B
  module ClassMethods; end
  mixes_in_class_methods(ClassMethods)
  mixes_in_class_methods(ClassMethods) # explicitly not an error
end
