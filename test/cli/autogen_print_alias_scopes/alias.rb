# typed: strict

module Foo
  Alias = Bar

  # this ref should have name = Foo::OtherAlias::Alias, resolved = Foo::OtherAlias::Alias
  DeepAlias = OtherAlias::Alias
end
