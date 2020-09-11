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
  module ClassMethods; end
  extend T::Helpers

  mixes_in_class_methods(ClassMethods)
  mixes_in_class_methods(ClassMethods) # explicitly not an error
end
