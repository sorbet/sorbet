# typed: true
# frozen_string_literal: true

require_relative './constant__class_definition.rb'

sig { params(foo: Foo::Foo).returns(Foo::Foo) }
def foo(foo); end

class Baz
#     ^ apply-rename: [D] newName: Bar invalid: true expectedErrorMessage: Renaming constants defined in .rbi files is not supported; symbol Baz is defined at test/testdata/lsp/rename/constant__rbi_class_reference.rbi:9

end
