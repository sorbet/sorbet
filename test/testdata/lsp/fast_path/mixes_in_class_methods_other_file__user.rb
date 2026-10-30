# typed: true
# spacer for exclude-from-file-update

class A
  include Mixin
end

A.hello # error: Method `hello` does not exist
