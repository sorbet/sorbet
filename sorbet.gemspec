Gem::Specification.new do |s|
  s.name        = 'sorbet'
  s.version     = '0.0.1'
  s.summary     = 'A Typechecker for Ruby'
  s.description = 'The main entrypoint for the Sorbet suite of gems. It depends on many other gems which provide some of the sub commands'
  s.authors     = ['Stripe']
  s.files       = Dir.glob('lib/**/*')
  s.executables = Dir.glob('bin/**/*').map {|path| path.gsub('bin/', '')}
  s.homepage    = 'https://sorbet.run'

  s.add_dependency('sorbet-rbi-generation')
  s.add_dependency('rbis')

  # The binary `sorbet` is included, so it is platform dependent
  s.platform = Gem::Platform::CURRENT

  # TODO this is temporary, to prevent leaking publicly.
  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  raise 'RubyGems 2.0 or newer is required to protect against public gem pushes.' unless s.respond_to?(:metadata)
  s.metadata['allowed_push_host'] = ''
end
