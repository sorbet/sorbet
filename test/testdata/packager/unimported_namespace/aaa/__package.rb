# frozen_string_literal: true
# typed: strict
# enable-packager: true

class AAA < PackageSpec # error: Package `AAA` is missing imports
  export AAA::AClass
end
