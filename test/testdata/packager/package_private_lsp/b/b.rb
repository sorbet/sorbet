# typed: true
module Root::B
  Root::A::Secret
# ^^^^^^^^^^^^^^^ error: `Root::A::Secret` is `package_private!` in package `Root::A`
  Root::A::X
# ^^^^^^^^^^ error: `Root::A::X` is `package_private!` in package `Root::A`
end
