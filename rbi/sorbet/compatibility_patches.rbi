# typed: true

# These modules provide optional runtime compatibility patches to fix
# behaviour modified by sorbet-runtime

module T::CompatibilityPatches
end

module T::CompatibilityPatches::MethodExtensions
  def arity; end
  def source_location; end
  def parameters; end
end
