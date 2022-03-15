# frozen_string_literal: true
# typed: true

require 'json'
require 'optparse'
require 'set'
require 'tempfile'

require 'sorbet-runtime'

class Module
  include T::Sig
end

class PackageInfo
  sig {returns(String)}
  attr_reader :name

  sig {returns(T::Array[String])}
  attr_reader :imports

  sig {returns(T::Array[String])}
  attr_reader :testImports

  sig {returns(T::Array[String])}
  attr_reader :files

  sig {returns(T::Array[String])}
  attr_reader :testFiles

  sig {returns(String)}
  attr_reader :mangled_name

  sig {params(hash: T::Hash[T.untyped, T.untyped]).returns(PackageInfo)}
  def self.from_hash(hash)
    self.new(hash.fetch("name"),
                    hash.fetch("imports"),
                    hash.fetch("testImports"),
                    hash.fetch("files"),
                    hash.fetch("testFiles"))
  end

  private_class_method :new

  sig {params(
         name: String,
         imports: T::Array[String],
         testImports: T::Array[String],
         files: T::Array[String],
         testFiles: T::Array[String]).void}
  def initialize(name, imports, testImports, files, testFiles)
    @name = name
    @imports = imports
    @testImports = testImports
    @files = files
    @testFiles = testFiles

    @mangled_name = T.let("#{name.gsub('::', '_')}_Package", String)
  end
end

class DependencyInfo < T::Struct
  const :packageRefs, T::Array[String]
  const :rbiRefs, T::Array[String]

  def self.load(file)
    from_hash(JSON.parse(File.read(file)))
  end
end

class PackageDB
  sig {returns(T::Hash[String, String])}
  attr_reader :rbis

  sig {returns(T::Hash[String, DependencyInfo])}
  attr_reader :deps

  sig {returns(T::Hash[String, PackageInfo])}
  attr_reader :info

  sig {params(package_dir: String, package_info: T::Array[PackageInfo]).void}
  def initialize(package_dir, package_info)
    @rbis = T.let({}, T::Hash[String, String])
    @deps = T.let({}, T::Hash[String, DependencyInfo])
    @info = T.let(package_info.to_h {|info| [info.name, info]}, T::Hash[String, PackageInfo])

    Dir.new(package_dir).each do |entry|
      next if entry == '.' or entry == '..'

      case
      when entry =~ /([^.]+).deps.json$/
        @deps[$1] = DependencyInfo.load(File.join(package_dir, entry))
      when entry =~ /([^.]+).package.rbi$/
        @rbis[$1] = entry
      else
        raise "Unknown filename in package directory #{entry}"
      end
    end
  end

  sig {params(info: PackageInfo).returns(String)}
  def lookup_rbi_for(info)
    rbis.fetch(info.mangled_name)
  end
  
  sig {params(info: PackageInfo).returns(DependencyInfo)}
  def lookup_deps_for(info)
    deps.fetch(info.mangled_name)
  end

  sig {params(name: String).returns(PackageInfo)}
  def lookup_package_for(name)
    info.fetch(name)
  end
end

module Runner
  include T::Sig

  sig {params(command: T::Array[String], chdir: T.nilable(String)).void}
  def self.check_call(command, chdir: nil)
    opts = {}
    if chdir
      opts[:chdir] = chdir
    end
    pid = T.unsafe(Process).spawn({}, *command, opts)
    Process.wait(pid)
    status = $?
    if status.to_i != 0
      raise "#{command.join(" ")} exited with #{status.to_i}"
    end
  end

  sig {params(package_info_file: Tempfile).returns(T::Array[PackageInfo])}
  def self.load_package_info(package_info_file)
    json = JSON.parse(File.read(package_info_file))

    json.map do |package|
      p = PackageInfo.from_hash(package)
    end
  end

  sig {params(sorbet: String, root: String, test_directory: String,
              required_files: T::Set[String], rbi_package_dir: String,
              rbi_files: T::Set[String]).void}
  def self.verify_single_package_typechecking(
        sorbet, root, test_directory, required_files,
        rbi_package_dir, rbi_files)
    Dir.mktmpdir do |tmpdir|
      # Copy the package's Ruby files to the tmpdir, mirroring the directory
      # structure.
      dirnames = required_files.map {|f| File.dirname(f)}.uniq
      dirnames.each do |dirname|
        target = File.join(tmpdir, dirname)
        FileUtils.mkdir_p(target)
      end

      Dir.chdir(root) do |root|
        required_files.each do |f|
          dirname = File.dirname(f)
          target = File.join(tmpdir, dirname)
          FileUtils.cp(f, target)
        end
      end

      # Copy the RBI files into the root.
      rbi_files.each do |rbi|
        source = File.join(rbi_package_dir, rbi)
        FileUtils.cp(source, File.join(tmpdir, test_directory))
      end

      check_call([
                   sorbet,
                   '--silence-dev-message',
                   '--stripe-mode',
                   '--ignore',
                   '__package.rb',
                   test_directory
                 ],
                 chdir: tmpdir)
    end
  end

  sig {params(sorbet: String, root: String, test_directory: String).void}
  def self.main(sorbet:, root:, test_directory:)
    Tempfile.open('package_info') do |package_info_file|
      check_call([
                   sorbet,
                   '--silence-dev-message',
                   '--stripe-mode',
                   '--stripe-packages',
                   '--dump-package-info',
                   T.must(package_info_file.path),
                   test_directory
                 ], chdir: root)
      
      package_info = load_package_info(package_info_file)

      package_directories = package_info.map do |package|
        dirs = package.files.map {|f| File.dirname(f)}.uniq
        if dirs.size != 1 
          raise "Package #{package.name} must be limited to a single directory"
        end

        dirs[0]
      end
      
      Dir.mktmpdir do |rbi_package_dir|
        check_call([
                     sorbet,
                     '--silence-dev-message',
                     '--stripe-mode',
                     '--package-rbi-generation',
                     '--package-rbi-dir',
                     rbi_package_dir,
                     '--ignore',
                     '__package.rb',
                     test_directory],
                   chdir: root)

        db = PackageDB.new(rbi_package_dir, package_info)

        package_info.map do |info|
          required_files = T.let(Set.new, T::Set[String])
          rbi_files = T.let(Set.new, T::Set[String])
          visited = T.let(Set.new, T::Set[String])
          worklist = T.let([], T::Array[String])

          # We always need the rb files from the package itself, but we should only
          # need the RBI files from any transitive imports.
          required_files.merge(info.files)
          required_files.merge(info.testFiles)
          visited.add(info.name)
          worklist.append(*info.imports)
          worklist.append(*info.testImports)

          while !worklist.empty?
            pkg = T.must(worklist.pop)
            next if visited.include?(pkg)

            visited.add(pkg)
            info_for_dependency = db.lookup_package_for(pkg)

            rbi_files.add(db.lookup_rbi_for(info_for_dependency))

            worklist.append(*info_for_dependency.imports)
            worklist.append(*info_for_dependency.testImports)
          end

          verify_single_package_typechecking(sorbet, root, test_directory, required_files,
                                             rbi_package_dir, rbi_files)
        end
      end
    end
  end
end

if __FILE__ == $0
  sorbet = T.let(nil, T.nilable(String))
  root = T.let(nil, T.nilable(String))
  test_directory = T.let(nil, T.nilable(String))

  OptionParser.new do |opts|
    opts.on '--sorbet=PATH', 'Path to the Sorbet executable' do |path|
      sorbet = File.realpath(path)
    end

    opts.on '--root=PATH', 'Path to the root of the Sorbet checkout' do |path|
      root = File.realpath(path)
    end

    opts.on '--test-directory=PATH', 'Relative path to the root of the checked directory' do |path|
      test_directory = path
    end
  end.parse!

  Runner.main(
    sorbet: T.must(sorbet),
    root: T.must(root),
    test_directory: T.must(test_directory)
  )
end
