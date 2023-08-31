# typed: true
# I'm fine with the weird <static-init> error,
# because the "outside of method" error will fire first.
  super
# ^^^^^ error: `super` outside of method
# ^^^^^ error: `<static-init>` does not exist on ancestors of
