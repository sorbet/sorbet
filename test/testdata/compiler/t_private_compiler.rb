# frozen_string_literal: true
# typed: true
# compiled: true

running_compiled = T::Private::Compiler.running_compiled?
case running_compiled
when false, true
  puts 'ok: T::Private::Compiler.running_compiled?'
else
  T.absurd(running_compiled)
end

compiler_version = T::Private::Compiler.compiler_version
if running_compiled && compiler_version.is_a?(String)
  puts 'ok: T::Private::Compiler.compiler_version'
elsif !running_compiled && compiler_version.nil?
  puts 'ok: T::Private::Compiler.compiler_version'
else
  raise "compiler_version does not match running_compiled?"
end
