# typed: strict

module Root::B
  package_private!
# ^^^^^^^^^^^^^^^^ error: `package_private!` cannot mark the root namespace of package `Root::B`
end
