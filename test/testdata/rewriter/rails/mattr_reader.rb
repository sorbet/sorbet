# typed: strict

class GoodUsages
  extend T::Sig
  mattr_reader :both, :foo
  mattr_reader :no_instance, instance_accessor: false
  mattr_reader :bar, :no_instance_reader, instance_reader: false

  sig {void}
  def usages
    both
    no_instance # error: Method `no_instance` does not exist
    no_instance_reader # error: Method `no_instance_reader` does not exist
  end

  both
  no_instance
  no_instance_reader
end

class IgnoredUsages
  mattr_reader # error: Method `mattr_reader` does not exist
  mattr_reader instance_accessor: false # error: Method `mattr_reader` does not exist
  mattr_reader "foo" # error: Method `mattr_reader` does not exist
end
