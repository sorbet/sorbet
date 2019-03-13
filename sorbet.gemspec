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

  # The binary `sorbet` is included, so it is platform dependent
  s.platform = Gem::Platform::CURRENT

  s.post_install_message = %q{
  Thanks for installing Sorbet! To use it in your project, first run:

    srb init

  which should make all sorts of `.rbi` shims for you as well as include the `rbis` gem in your Gemfile.
  After that, whenever you want to typecheck your code use:

    srb tc

  For more docs see: https://sorbet.tech
  }

  # TODO this is temporary, to prevent leaking publicly.
  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  raise 'RubyGems 2.0 or newer is required to protect against public gem pushes.' unless s.respond_to?(:metadata)
  s.metadata['allowed_push_host'] = ''
end
