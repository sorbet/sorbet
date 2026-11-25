# typed: strict
# enable-packager: true

# `export` and `export_all!` are not package directives. Constants are public unless marked `package_private!`.
class Root < PackageSpec
  export Root::Thing
# ^^^^^^ error: Method `export` does not exist on `T.class_of(Root)`
  #      ^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals

  export_all!
# ^^^^^^^^^^^ error: Method `export_all!` does not exist on `T.class_of(Root)`
end
