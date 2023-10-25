# typed: strict

class GoodUsages
  extend T::Sig
  cattr_writer :both, :foo
  cattr_writer :no_instance, instance_accessor: false
  cattr_writer :bar, :no_instance_writer, instance_writer: false

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
  cattr_writer # error: Method `cattr_writer` does not exist
  cattr_writer instance_accessor: false # error: Method `cattr_writer` does not exist
  cattr_writer "foo" # error: Method `cattr_writer` does not exist
end
