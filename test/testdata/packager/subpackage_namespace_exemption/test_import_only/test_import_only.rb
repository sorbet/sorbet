# typed: true

module TestImportOnly
  extend T::Generic # error: `T.class_of(TestImportOnly)` can only have ancestors if every package nested under `TestImportOnly` imports it
  Elem = type_member # error: `TestImportOnly` can only be generic if every package nested under `TestImportOnly` imports it
end
