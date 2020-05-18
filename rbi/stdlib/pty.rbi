# typed: __STDLIB_INTERNAL

# Creates and manages pseudo terminals (PTYs). See also
# http://en.wikipedia.org/wiki/Pseudo\_terminal
#
# [`PTY`](https://docs.ruby-lang.org/en/2.6.0/PTY.html) allows you to allocate
# new terminals using
# [`::open`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-open) or
# [`::spawn`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-spawn) a new
# terminal with a specific command.
#
# ## Example
#
# In this example we will change the buffering type in the `factor` command,
# assuming that factor uses stdio for stdout buffering.
#
# If [`IO.pipe`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-pipe) is
# used instead of
# [`PTY.open`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-open), this
# code deadlocks because factor's stdout is fully buffered.
#
# ```ruby
# # start by requiring the standard library PTY
# require 'pty'
#
# master, slave = PTY.open
# read, write = IO.pipe
# pid = spawn("factor", :in=>read, :out=>slave)
# read.close     # we dont need the read
# slave.close    # or the slave
#
# # pipe "42" to the factor command
# write.puts "42"
# # output the response from factor
# p master.gets #=> "42: 2 3 7\n"
#
# # pipe "144" to factor and print out the response
# write.puts "144"
# p master.gets #=> "144: 2 2 2 2 3 3\n"
# write.close # close the pipe
#
# # The result of read operation when pty slave is closed is platform
# # dependent.
# ret = begin
#         master.gets     # FreeBSD returns nil.
#       rescue Errno::EIO # GNU/Linux raises EIO.
#         nil
#       end
# p ret #=> nil
# ```
#
# ## License
#
# ```
# C) Copyright 1998 by Akinori Ito.
#
# This software may be redistributed freely for this purpose, in full
# or in part, provided that this entire copyright notice is included
# on any copies of this software and applications and derivations thereof.
#
# This software is provided on an "as is" basis, without warranty of any
# kind, either expressed or implied, as to any matter including, but not
# limited to warranty of fitness of purpose, or merchantability, or
# results obtained from use of this software.
# ```
module PTY
  # Checks the status of the child process specified by `pid`. Returns `nil` if
  # the process is still alive.
  #
  # If the process is not alive, and `raise` was true, a
  # [`PTY::ChildExited`](https://docs.ruby-lang.org/en/2.6.0/PTY/ChildExited.html)
  # exception will be raised. Otherwise it will return a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.6.0/Process/Status.html)
  # instance.
  #
  # `pid`
  # :   The process id of the process to check
  # `raise`
  # :   If `true` and the process identified by `pid` is no longer alive a
  #     [`PTY::ChildExited`](https://docs.ruby-lang.org/en/2.6.0/PTY/ChildExited.html)
  #     is raised.
  def self.check(*_); end

  # Spawns the specified command on a newly allocated pty. You can also use the
  # alias
  # [`::getpty`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-getpty).
  #
  # The command's controlling tty is set to the slave device of the pty and its
  # standard input/output/error is redirected to the slave device.
  #
  # `command` and `command_line` are the full commands to run, given a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). Any additional
  # `arguments` will be passed to the command.
  #
  # ### Return values
  #
  # In the non-block form this returns an array of size three, `[r, w, pid]`.
  #
  # In the block form these same values will be yielded to the block:
  #
  # `r`
  # :   A readable [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) that
  #     contains the command's standard output and standard error
  # `w`
  # :   A writable [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) that is
  #     the command's standard input
  # `pid`
  # :   The process identifier for the command.
  def self.getpty(*_); end

  # Allocates a pty (pseudo-terminal).
  #
  # In the block form, yields two arguments `master_io, slave_file` and the
  # value of the block is returned from `open`.
  #
  # The [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and
  # [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) are both closed
  # after the block completes if they haven't been already closed.
  #
  # ```ruby
  # PTY.open {|master, slave|
  #   p master      #=> #<IO:masterpty:/dev/pts/1>
  #   p slave      #=> #<File:/dev/pts/1>
  #   p slave.path #=> "/dev/pts/1"
  # }
  # ```
  #
  # In the non-block form, returns a two element array, `[master_io,
  # slave_file]`.
  #
  # ```ruby
  # master, slave = PTY.open
  # # do something with master for IO, or the slave file
  # ```
  #
  # The arguments in both forms are:
  #
  # `master_io`
  # :   the master of the pty, as an
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html).
  # `slave_file`
  # :   the slave of the pty, as a
  #     [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html). The path to the
  #     terminal device is available via `slave_file.path`
  #
  #
  # [`IO#raw!`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-raw-21) is
  # usable to disable newline conversions:
  #
  # ```
  # require 'io/console'
  # PTY.open {|m, s|
  #   s.raw!
  #   ...
  # }
  # ```
  def self.open; end

  # Spawns the specified command on a newly allocated pty. You can also use the
  # alias
  # [`::getpty`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-getpty).
  #
  # The command's controlling tty is set to the slave device of the pty and its
  # standard input/output/error is redirected to the slave device.
  #
  # `command` and `command_line` are the full commands to run, given a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). Any additional
  # `arguments` will be passed to the command.
  #
  # ### Return values
  #
  # In the non-block form this returns an array of size three, `[r, w, pid]`.
  #
  # In the block form these same values will be yielded to the block:
  #
  # `r`
  # :   A readable [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) that
  #     contains the command's standard output and standard error
  # `w`
  # :   A writable [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) that is
  #     the command's standard input
  # `pid`
  # :   The process identifier for the command.
  def self.spawn(*_); end
end

# Thrown when
# [`PTY::check`](https://docs.ruby-lang.org/en/2.6.0/PTY.html#method-c-check) is
# called for a pid that represents a process that has exited.
class PTY::ChildExited < ::RuntimeError
  # Returns the exit status of the child for which PTY#check raised this
  # exception
  def status; end
end
