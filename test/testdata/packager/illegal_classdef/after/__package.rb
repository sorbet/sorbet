# frozen_string_literal: true
# typed: strict
# enable-packager: true

class After < PackageSpec
end

class A; end # error: Invalid expression in package: `ClassDef` not allowed
