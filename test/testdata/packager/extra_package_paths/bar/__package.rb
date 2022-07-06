# frozen_string_literal: true

# typed: strict
# enable-packager: true
# extra-package-files-directory-prefix-underscore: test/testdata/packager/extra_package_paths/extra/
# extra-package-files-directory-prefix-slash: test/testdata/packager/extra_package_paths/extra_slash/

class Project::Bar < PackageSpec
  import Project::Foo
  import Project::Baz::Package
  import Project::FooBar
end
