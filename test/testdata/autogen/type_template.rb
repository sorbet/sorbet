# typed: true
# rubocop:disable PrisonGuard/AutogenLoaderPreamble, PrisonGuard/ModuleOpus, PrisonGuard/NoTopLevelDeclarations
module Foo
  class Bar
    extend T::Generic
    X = type_template
    Y = type_template
    Z = type_template { {fixed: Integer} }
  end
end
