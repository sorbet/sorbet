# typed: strict

class GoodUsages
  extend T::Sig
  mattr_accessor :both, :foo
  mattr_accessor :no_instance, instance_accessor: false
  mattr_accessor :no_instance_reader, instance_reader: false
  mattr_accessor :bar, :no_instance_writer, instance_writer: false

  sig {void}
  def usages
    both
    self.both = 1

    no_instance # error: Method `no_instance` does not exist
    self.no_instance = 1 # error: Setter method `no_instance=` does not exist

    no_instance_reader # error: Method `no_instance_reader` does not exist
    self.no_instance_reader= 1

    no_instance_writer
    self.no_instance_writer = 1 # error: Setter method `no_instance_writer=` does not exist
  end

  both
  self.both = 1

  no_instance
  self.no_instance = 1

  no_instance_reader
  self.no_instance_reader = 1

  no_instance_writer
  self.no_instance_writer = 1
end

class IgnoredUsages
  mattr_accessor # error: Method `mattr_accessor` does not exist
  mattr_accessor instance_accessor: false # error: Method `mattr_accessor` does not exist
  mattr_accessor "foo" # error: Method `mattr_accessor` does not exist
end
