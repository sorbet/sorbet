# typed: strict

class MyPkg < PackageSpec
  custom_method 'abc'
  bad_method 'def'
# ^^^^^^^^^^^^^^^^ error: Method `bad_method` does not exist on `T.class_of(MyPkg)`
end
