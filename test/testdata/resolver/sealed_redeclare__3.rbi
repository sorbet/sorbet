# typed: strict

class C
  extend T::Helpers
  # This is not an error at runtime because RBI files don't affect the runtime.
  #
  # But this does represent an order depencence bug statically, because if this
  # file is read before the source file, Sorbet will think that `C` can only be
  # inherited in this file.
  sealed!
end
