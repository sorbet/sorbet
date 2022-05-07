# frozen_string_literal: true
# typed: strict
# enable-packager: true

class RBIGen < PackageSpec
    export RBIGen::Public
    export Test::RBIGen
    export RBIGen::DirectlyExported::MyString
    export_for_test RBIGen::Private::PrivateClassForTests
  # ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
    #               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals

    # RBIGen::Sealed::Child as well as ...::Parent will go in the .package.rbi
    # file, while the definition of only ...::TestChild will end up in the
    # .test.package.rbi file
    export RBIGen::Sealed::Child
    export_for_test RBIGen::Sealed::TestChild
  # ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
    #               ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
end
