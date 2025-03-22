# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Inside < PackageSpec
  class A; end # error: Invalid expression in package: `ClassDef` not allowed
end
