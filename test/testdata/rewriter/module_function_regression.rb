# typed: true

# This guards against a regression in ModuleFunction.cc, where `prevStat` was
# assumed to be non-null.

module M
  module_function def foo; end
end
