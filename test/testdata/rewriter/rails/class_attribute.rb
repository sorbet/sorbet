# typed: strict

class GoodUsages
  extend T::Sig
  class_attribute :read_write_predicate, :foo
  class_attribute :no_instance, instance_accessor: false
  class_attribute :no_instance_reader, instance_reader: false
  class_attribute :no_instance_writer, instance_writer: false
  class_attribute :bar, :no_predicate, instance_predicate: false

  sig {void}
  def usages
    read_write_predicate
    read_write_predicate?
    self.read_write_predicate = 1

    no_instance # error: Method `no_instance` does not exist
    no_instance? # error: Method `no_instance?` does not exist
    self.no_instance = 1 # error: Setter method `no_instance=` does not exist

    no_instance_reader # error: Method `no_instance_reader` does not exist
    no_instance_reader? # error: Method `no_instance_reader?` does not exist
    self.no_instance_reader = 1

    no_instance_writer
    no_instance_writer?
    self.no_instance_writer = 1 # error: Setter method `no_instance_writer=` does not exist

    no_predicate
    no_predicate? # error: Method `no_predicate?` does not exist
    self.no_predicate = 1
  end

  read_write_predicate
  read_write_predicate?
  self.read_write_predicate = 1

  no_instance
  no_instance?
  self.no_instance = 1

  no_instance_reader
  no_instance_reader?
  self.no_instance_reader = 1

  no_instance_writer
  no_instance_writer?
  self.no_instance_writer = 1

  no_predicate
  no_predicate? # error: Method `no_predicate?` does not exist
  self.no_predicate = 1
end

class IgnoredUsages
  class_attribute # error: Method `class_attribute` does not exist
  class_attribute instance_accessor: false # error: Method `class_attribute` does not exist
  class_attribute "foo" # error: Method `class_attribute` does not exist
end

module NotAvailableInModule
  class_attribute :foo, :bar # error: Method `class_attribute` does not exist
end
