# typed: true

# This tests that we don't regress in LSP / incremental resolve mode.
#
# In incremental resolve mode, even if a class has only one call to
# mixes_in_class_methods, Sorbet sees it twice (once from the first pass, once
# from the second, incrementalResolve, pass).
#
# If the second one we see is the same as the one we saw previously, this is
# actually fine.

module A
  extend T::Helpers
  module ClassMethods1; end
  module ClassMethods2; end
  mixes_in_class_methods(ClassMethods1)
  mixes_in_class_methods(ClassMethods2)
end

module B
  extend T::Helpers
  module ClassMethods; end
  mixes_in_class_methods(ClassMethods)
  mixes_in_class_methods(ClassMethods) # explicitly not an error
end
