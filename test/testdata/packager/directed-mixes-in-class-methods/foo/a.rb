# typed: true
# spacer for exclude-from-file-update

class Root::Foo::A
  include Root::Mixin
end

Root::Foo::A.hello # error: Method `hello` does not exist
