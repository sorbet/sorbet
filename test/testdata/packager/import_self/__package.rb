# frozen_string_literal: true
# typed: strict
# enable-packager: true

class A < PackageSpec
  import A # error: Package `A` cannot import itself
end
