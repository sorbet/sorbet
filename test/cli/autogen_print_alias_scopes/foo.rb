# typed: strict

module Foo
  # this ref should have name = Foo::Alias::Cnst, resolved = Bar::Cnst
  Alias::Cnst

  # this ref should have name = Foo::DeepAlias::Cnst, resolved = Bar::Cnst
  DeepAlias::Cnst
end
