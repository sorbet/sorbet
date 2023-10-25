# typed: strict

class GoodUsages
  extend T::Sig
  thread_cattr_accessor :both, :foo
  thread_cattr_accessor :no_instance, instance_accessor: false
  thread_cattr_accessor :no_instance_reader, instance_reader: false
  thread_cattr_accessor :bar, :no_instance_writer, instance_writer: false

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
  thread_cattr_accessor # error: Method `thread_cattr_accessor` does not exist
  thread_cattr_accessor instance_accessor: false # error: Method `thread_cattr_accessor` does not exist
  thread_cattr_accessor "foo" # error: Method `thread_cattr_accessor` does not exist
end
