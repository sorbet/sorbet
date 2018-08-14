# This file contains constants and methods that are available for users to use
# in RBI files, so aren't present at runtime.

class Sorbet
  # Identical to `T::Helpers`'s `sig` in semantics, but couldn't work at
  # runtime since it doesn't know `self`. Used in `rbi`s that don't `extend
  # T::Helpers`.
  def self.sig(*args)
  end
end
