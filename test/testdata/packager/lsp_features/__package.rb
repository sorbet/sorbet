# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "(nothing)"
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
    #      ^^^^ symbol-search: "Bart", name = "Bart", container = "Simpsons"
    #      ^^^^ usage: bartpkg
    #      ^^^^ hover: Bart package description
    export Simpsons::Family
    #                ^^^^^^ usage: family
end
