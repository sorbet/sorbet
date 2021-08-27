# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "(nothing)"
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
    #      ^^^^ symbol-search: "Bart", name = "Bart", container = "Simpsons"
    #      ^^^^ def: bart 1 not-def-of-self
    #      ^^^^ usage: bartpkg
    #      ^^^^ hover: Bart package description

    test_import Krabappel
    #           ^^^^^^^^^ usage: krabappel-pkg
    #           ^^^^^^^^^ hover: Bart's teacher

    export Simpsons::Family
    #                ^^^^^^ usage: family
end
