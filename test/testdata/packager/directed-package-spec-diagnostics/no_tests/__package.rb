# typed: strict

class NoTests < PackageSpec
  layer 'a'
  strict_dependencies 'false'
  bad_method 'def'
# ^^^^^^^^^^ error: Method `bad_method` does not exist on `T.class_of(NoTests)`
end
