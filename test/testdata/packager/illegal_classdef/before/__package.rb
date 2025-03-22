# frozen_string_literal: true
# typed: strict
# enable-packager: true

class A; end # error-with-dupes: `__package.rb` file must contain a package definition

class Before < PackageSpec
end
