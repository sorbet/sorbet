# typed: true

module Second
# error: File belongs to package `First` but defines a constant that does not match this namespace
  class FromFirst # error: File belongs to package `First` but defines a constant that does not match this namespace
  end
end
