# typed: true

# Reject the source namespace regardless of whether its aliases have resolved.
class A::Same::Nested
end
A::Same::FIELD = 1

class A::Foreign::Nested
end
A::Foreign::FIELD = 1

class A::Chained::Nested
end
A::Chained::FIELD = 1

module A::ModuleAlias
  def alias_recovery_marker; end
end

# An unpackaged target must not make the foreign source namespace permissible.
class A::Unpackaged::Nested
end
A::Unpackaged::FIELD = 1
