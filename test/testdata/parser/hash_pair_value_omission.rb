# typed: true

[1,2,3].map do |x|
  { foo?: }
  # ^^^^^ error: identifier foo? is not valid to get
end
