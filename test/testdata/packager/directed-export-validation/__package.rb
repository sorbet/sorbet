# typed: strict
# enable-packager: true
# enable-package-directed: true
class Example < PackageSpec
  export Example::Outer::Inner # error: Cannot export `Example::Outer::Inner` because another exported name `Example::Outer` is a prefix of it
  export Example::Outer
  export Example::Outer # error: Duplicate export of `Example::Outer`
  export Example::Enum::Value # error: Cannot export enum value `Example::Enum::Value`. Instead, export the entire enum `Example::Enum`
  export Example::FromRBI # error: Cannot export `Example::FromRBI` because it is only defined in an RBI file
  export Example::Generic::Elem # error: Only classes, modules, or constants may be exported
  export Test::Example::Outer::Inner # error: Cannot export `Test::Example::Outer::Inner` because another exported name `Test::Example::Outer` is a prefix of it
  export Test::Example::Outer
  export Test::Example::Outer # error: Duplicate export of `Test::Example::Outer`
end
