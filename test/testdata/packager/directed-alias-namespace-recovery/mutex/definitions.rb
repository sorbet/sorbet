# typed: true

# The source spelling is on the package path, but the payload alias targets
# the unpackaged Thread::Mutex. Do not modify the target through the alias.
class Mutex # error: File belongs to package `Mutex` but defines a constant that does not match this namespace
  def alias_recovery_marker; end
end
