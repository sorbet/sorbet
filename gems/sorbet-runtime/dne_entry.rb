require_relative './lib/sorbet-runtime'

def (::RubyVM::InstructionSequence).load_iseq(path)
  return nil unless ENV["PATCH_LOAD_ISEQ"]
  return nil if path.end_with?('_transformed.rb')
  return nil unless path.end_with?('_untransformed.rb')

  transformed_src = File.read(path).gsub(/LAZY_CONSTANT = .*$/, "autoload :LAZY_CONSTANT, '#{path.gsub(/_untransformed\.rb$/, "_transformed.rb")}'")
  puts("-- transformed_src for path=#{path} --", transformed_src, "-----")
  return nil if transformed_src.nil?
  RubyVM::InstructionSequence.compile(transformed_src, path)
end

puts '-- before require --'
require_relative './dne_untransformed.rb'

puts("LAZY_CONSTANT=#{DNE::LAZY_CONSTANT}")

puts '-- before DNE.main --'
DNE.main
