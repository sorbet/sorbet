# typed: true

module Root::B
  Root::A::SECRET
# ^^^^^^^^^^^^^^^ error: `Root::A::SECRET` is `package_private!` in package `Root::A`
end
