# typed: true
# spacer for exclude-from-file-update
module Root::B
  Root::A::Inner::X
# ^^^^^^^^^^^^^^^^^ error: Non-private reference to private constant `Root::A::Inner::X` referenced
end
