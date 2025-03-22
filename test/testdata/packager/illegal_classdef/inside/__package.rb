# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Inside < PackageSpec
  class A; end # error: Package files can only declare one package
end
