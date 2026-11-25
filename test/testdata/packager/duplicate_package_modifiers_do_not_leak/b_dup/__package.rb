# typed: strict

# ...and this redefinition must not undo that.
class B < PackageSpec; end # error: Redefinition of package `B`
