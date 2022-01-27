# frozen_string_literal: true

# typed: strict
# enable-packager: true
# extra-package-files-directory-prefix: test/testdata/packager/extra_package_paths/extra/

class Project::Bar < PackageSpec
  import Project::Foo
  import Project::Baz::Package
end
