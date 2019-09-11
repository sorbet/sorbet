# typed: strict

class GoodUsages
  extend T::Sig
  cattr_reader :both, :foo
  cattr_reader :no_instance, instance_accessor: false
  cattr_reader :bar, :no_reader, instance_reader: false

  sig {void}
  def usages
    both
    no_instance # error: Method `no_instance` does not exist
    no_reader # error: Method `no_reader` does not exist
  end

  both
  no_instance
  no_reader
end

class IgnoredUsages
  cattr_reader # error: Method `cattr_reader` does not exist
  cattr_reader instance_accessor: false # error: Method `cattr_reader` does not exist
  cattr_reader "foo" # error: Method `cattr_reader` does not exist
end
