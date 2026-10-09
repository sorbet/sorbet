# typed: true

# The source spelling is on the package path, but the payload alias targets
# the unpackaged Thread::Mutex. Do not modify the target through the alias.
class Mutex # error: Redefining constant `Mutex` as a class or module
  def alias_recovery_marker; end
end
