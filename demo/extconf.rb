require 'mkmf'

RbConfig::MAKEFILE_CONFIG['cleanobjs'] = '*.ll' # nuke ll files in clean


# force usage of clang
ENV['CC'] ||= find_executable('clang')
ENV['CXX'] ||= find_executable('clang++')
RbConfig::MAKEFILE_CONFIG["CC"] = ENV["CC"] if ENV["CC"]
RbConfig::MAKEFILE_CONFIG["CXX"] = ENV["CXX"] if ENV["CXX"]

create_header
create_makefile 'foobar'

File.open('Makefile', 'at') do |mk|
    mk.print <<EOF

### CUSTOM MODIFICATIONS
%.ll : %.c
	$(ECHO) building LLVM bitcode for $(<) into $@
	$(Q) $(CC) -S -emit-llvm $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) $(COUTFLAG)$@ -O0 $(CSRCFLAG)$<
EOF
end
