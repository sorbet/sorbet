# typed: strict

class GoodUsages
  extend T::Sig
  mattr_writer :both, :foo
  mattr_writer :no_instance, instance_accessor: false
  mattr_writer :bar, :no_instance_writer, instance_writer: false

  sig {void}
  def usages
    self.both = 1
    self.no_instance = 1 # error: Setter method `no_instance=` does not exist
    self.no_instance_writer = 1 # error: Setter method `no_instance_writer=` does not exist
  end

  self.both = 1
  self.no_instance = 1
  self.no_instance_writer = 1
end

class IgnoredUsages
  mattr_writer # error: Method `mattr_writer` does not exist
  mattr_writer instance_accessor: false # error: Method `mattr_writer` does not exist
  mattr_writer "foo" # error: Method `mattr_writer` does not exist
end
