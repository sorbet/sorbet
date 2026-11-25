# typed: true
# spacer for exclude-from-file-update
module Root::B
  Root::A::Inner::Y
# ^^^^^^^^^^^^^^^^^ error: Non-private reference to private constant `Root::A::Inner::Y` referenced
end
