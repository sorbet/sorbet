# typed: strict
# enable-packager: true
# enable-package-directed: true
class Example < PackageSpec
  export Example::Outer::Inner
  export Example::Outer
  export Example::Outer
  export Example::Enum::Value
  export Example::FromRBI
  export Example::Generic::Elem
end
