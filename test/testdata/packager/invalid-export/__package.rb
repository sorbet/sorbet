# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec

  export Foo::Bar::Exists
  export Foo::Bar::NotDefined

  export Foo::Bar::HasTypeMember::TypeMember

end
