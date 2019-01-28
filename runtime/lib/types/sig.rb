# frozen_string_literal: true
# typed: true
# rubocop:disable PrisonGuard/NoTopLevelDeclarations, PrisonGuard/PackageMatchesDirectory

# Monkeypatches standard objects so that you can call `sig` anywhere.
# Docs at http://go/types
module T::Sig
  # Declares a method with type signatures and/or
  # abstract/override/... helpers. See the documentation URL on
  # {T::Helpers}
  def sig(&blk) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
    T::Private::Methods.declare_sig(self, &blk)
  end
end

class Module
  include T::Sig
end

# rubocop:enable PrisonGuard/NoTopLevelDeclarations, PrisonGuard/PackageMatchesDirectory
