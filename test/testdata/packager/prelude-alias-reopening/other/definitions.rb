# typed: true

# A rejected reopening must not mask the real alias in another file.
class Queue # error: File belongs to package `Other` but defines a constant that does not match this namespace
  def alias_recovery_marker; end
end
