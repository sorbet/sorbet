# typed: true

# Reject the source namespace regardless of whether its aliases have resolved.
class A::Same::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
end
A::Same::FIELD = 1 # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace

class A::Foreign::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
end
A::Foreign::FIELD = 1 # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace

class A::Chained::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
end
A::Chained::FIELD = 1 # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace

module A::ModuleAlias # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
  def alias_recovery_marker; end
end

# An unpackaged target must not make the foreign source namespace permissible.
class A::Unpackaged::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
end
A::Unpackaged::FIELD = 1 # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
