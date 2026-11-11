# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec
  export_all!

  export_all!(:with_argument)
            # ^^^^^^^^^^^^^^ error: Too many arguments

  export Foo::Bar::Thing

end
