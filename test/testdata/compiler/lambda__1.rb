# frozen_string_literal: true
# typed: true
# compiled: true

# lambda__2.rb makes a lambda and stores it in a top-level constant. Calling it
# after requiring that file ensures that all of its locals would have exited the
# scope of the C stack, so that we know we're relying on the VMs functionality
# for moving the environment into the heap.
require_relative './lambda__2.rb'

Main::Foo.call('test')

OtherMain::Bar.call
