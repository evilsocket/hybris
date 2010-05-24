require 'rake'
require 'rake/clean'
require 'mkmf'

verbose(false)

TARGET  	  = 'hybris'
RELEASE 	  = '1.0b3'

PREFIX        = "/usr"
FLEX		  = 'flex'
BISON		  = 'bison'
CXX           = 'g++'
WFLAGS        = '-w'
OPTIMIZATION  = '-O3 -pipe -fomit-frame-pointer -ffast-math'
CXXFLAGS      = "-Iinclude/ #{WFLAGS} #{OPTIMIZATION}" 
LDFLAGS       = "-L./build#{PREFIX}/lib/ -ldl -lpcre -lpthread -lhybris"
LIBXML_CFLAGS = "`xml2-config --cflags`"
LIBXML_LFLAGS = "`xml2-config --libs`"
LIBFFI_CFLAGS = "`pkg-config libffi --cflags`"
LIBFFI_LFLAGS = "`pkg-config libffi --libs`"
STDLIB_LFLAGS = "-ldl -lpcre -lcurl -lpthread -lreadline #{LIBXML_LFLAGS} #{LIBFFI_LFLAGS}" 
STDLIB_CFLAGS = "#{WFLAGS} -L. -L./build#{PREFIX}/lib/ -I./include/ #{OPTIMIZATION} -fPIC #{LIBXML_CFLAGS} #{LIBFFI_CFLAGS} -lhybris -lc -shared"


CLEAN.include( 'src/**/*.o', 
               'src/**/*.lo', 
               TARGET, 
			   'include/config.h',
               'src/parser.cpp', 'src/lexer.cpp', 'include/parser.h', 
			   'build' )

#----------------------------- Files ----------------------------

SRC = FileList['src/**/*.cpp'] - ['src/lexer.l.cpp', 'src/parser.y.cpp'] + ['src/parser.cpp', 'src/lexer.cpp']

SOURCES = {
    :BIN => FileList['src/main.cpp'],
    :LIB => SRC   - ['src/main.cpp'],
    :STD => FileList['stdlib/**/*.cc']
}

OBJECTS = {
	:BIN => ['src/parser.cpp', 'src/lexer.cpp'] + SOURCES[:BIN].ext('o'),
	:LIB => ['src/parser.cpp', 'src/lexer.cpp'] + SOURCES[:LIB].ext('lo'),
	:STD => SOURCES[:STD].pathmap("%{^stdlib,build#{PREFIX}/lib/hybris/library/}X.so")
}

#----------------------------- Tasks ----------------------------

task :interpreter => TARGET
task :library     => "lib#{TARGET}.so.1.0"
task :modules     => OBJECTS[:STD]

task :exec    => [ :checkdeps, :config_h, :library, :interpreter ]
task :all     => [ :exec, :modules ]
task :debug do
	CXXFLAGS = "-Iinclude/ #{WFLAGS} -g -pg" 
	STDLIB_CFLAGS = "#{WFLAGS} -L. -L./build#{PREFIX}/lib/ -I./include/ -g -pg -fPIC #{LIBXML_CFLAGS} #{LIBFFI_CFLAGS} -lhybris -lc -shared"
	Rake::Task["all"].invoke
end
task :default => [ :all ]

task :checkdeps do
	raise "[ERROR] #{CXX} not found !\n" unless find_executable( CXX )
	raise "[ERROR] #{FLEX} not found !\n" unless find_executable( FLEX )
	raise "[ERROR] #{BISON} not found !\n" unless find_executable( BISON )
	raise "[ERROR] libdl not found !\n" unless have_library( "dl", "dlsym" )
	raise "[ERROR] dlfcn.h not found !\n" unless have_header( "dlfcn.h" )
	raise "[ERROR] libpcre not found !\n" unless have_library( "pcre", "pcre_compile" )
	raise "[ERROR] pcre.h not found !\n" unless have_header( "pcre.h" )
	raise "[ERROR] libpthread not found !\n" unless have_library( "pthread", "pthread_create" ) 
	raise "[ERROR] pthread.h not found !\n" unless have_header( "pthread.h" )
	raise "[ERROR] libffi not found !\n" unless have_library( "ffi", "ffi_call" ) 
	raise "[ERROR] libxml2 not found !\n" unless have_library( "xml2", "xmlReadFile" )
	raise "[ERROR] libcurl not found !\n" unless have_library( "curl", "curl_easy_init" )
	raise "[ERROR] curl.h not found !\n" unless have_header( "curl/curl.h" )
	raise "[ERROR] libreadline not found !\n" unless have_library( "readline", "readline" )
	raise "[ERROR] readline.h not found !\n" unless have_header( "readline/readline.h" )
end

task :config_h do
	puts "@ Creating config.h"
	File.open("include/config.h", "w+") do |config|
	      config.puts "
#define AUTHOR \"The Hybris Dev Team http://www.hybris-lang.org/\"
/* system path for hybris global scripts */
#define INC_PATH \"#{PREFIX}/lib/hybris/include/\"
/* system path for libraries (.so dynamic libs) */
#define LIB_PATH \"#{PREFIX}/lib/hybris/library/\"
/* Define to the version of this package. */
#define VERSION \"#{RELEASE}\""
	end

end

task :deb => :all do
	raise "[ERROR] You need to run this task on a debian environment !\n" unless File.exist?("/etc/debian_version")
	raise "[ERROR] You need dpkg-deb binary to run this task !\n" unless find_executable( "dpkg-deb" )
	# include files
	sh "mkdir -p build/#{PREFIX}/include/#{TARGET}"
	sh "cp -rf include/*.h build/#{PREFIX}/include/#{TARGET}/"
 	# directories and library
	sh "mkdir -p build/#{PREFIX}/lib/#{TARGET}/include"
	sh "cp -rf stdinc/* build/#{PREFIX}/lib/#{TARGET}/include"
	# debian control file
	sh "mkdir -p build/DEBIAN"
	arch = `dpkg --print-architecture`.chop()
	time = `date +'%Y%m%d%H%M%S'`.chop()
	File.open("build/DEBIAN/control", "w+") do |control|
	      control.puts "Package: #{TARGET}
Version: #{RELEASE}-#{time}
Architecture: #{arch}
Maintainer: Simone Margaritelli < evilsocket@gmail.com >
Installed-Size: 2512
Priority: optional
Pre-Depends: dpkg
Depends: libpcre3, libstdc++6, libcurl3, libffi5, libxml2
Section: developement
Description: Hybris programming language interpreter and standard libraries.
 Hybris, which stands for hybrid scripiting language, is an open source scripting language with dynamic typing, which is a language that does not require explicit declaration of the type of a variable, but understands how to treat the variable in question in accordance with the value which is initialized and subsequently treated, and object-oriented, which listen to the needs of developers who want to automate certain procedures in a simple and fast way."
	end
	sh "dpkg-deb -b ./build ."
end

task :install do
	raise '[ERROR] Must run as root' unless Process.uid == 0
	# binary
	sh "install -m 0755  build#{PREFIX}/bin/#{TARGET} #{PREFIX}/bin/"
	# include files
	sh "mkdir -p #{PREFIX}/include/#{TARGET}"
	sh "cp include/*.h #{PREFIX}/include/#{TARGET}/"
 	# directories and library
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}"
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}/include"
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}/library"
	sh "chmod -R 777 #{PREFIX}/lib/#{TARGET}/"
	sh "install -m 0644  build#{PREFIX}/lib/lib#{TARGET}.so.1.0 #{PREFIX}/lib"
	sh "ln -sf #{PREFIX}/lib/lib#{TARGET}.so.1.0 #{PREFIX}/lib/lib#{TARGET}.so"
	sh "ln -sf #{PREFIX}/lib/lib#{TARGET}.so.1.0 #{PREFIX}/lib/lib#{TARGET}.so.1"
	sh "ldconfig"
	# modules and std includes
	sh "cp -rf build/#{PREFIX}/lib/#{TARGET}/library/* #{PREFIX}/lib/#{TARGET}/library/"
	sh "cp -rf stdinc/* #{PREFIX}/lib/hybris/include/"
end

task :uninstall do
	raise '[ERROR]  Must run as root' unless Process.uid == 0

	sh "rm -rf #{PREFIX}/bin/#{TARGET} "
	sh "rm -rf #{PREFIX}/lib/#{TARGET}"
	sh "rm -rf #{PREFIX}/include/#{TARGET}"
	sh "rm -rf #{PREFIX}/lib/lib#{TARGET}.so.1.0"
	sh "rm -rf #{PREFIX}/lib/lib#{TARGET}.so"
	sh "rm -rf #{PREFIX}/lib/lib#{TARGET}.so.1"
end

#----------------------------- Compiling ----------------------------

rule 'src/lexer.cpp' => 'src/lexer.l.cpp' do |t|
	puts "@ Creating #{t.name}"
    sh "#{FLEX} -o#{t.name} #{t.source}"
end

rule 'src/parser.cpp' => 'src/parser.y.cpp' do |t|
	puts "@ Creating #{t.name}"
    sh "#{BISON} -y --defines=include/parser.h -o#{t.name} #{t.source}"
end

rule '.lo' => '.cpp' do |t|
	puts "@ Compiling #{t.source}"
    sh "#{CXX} #{CXXFLAGS} -fPIC -o #{t.name} -c #{t.source}"
end

rule '.o' => '.cpp' do |t|
	puts "@ Compiling #{t.source}"
    sh "#{CXX} #{CXXFLAGS} -o #{t.name} -c #{t.source}"
end

rule '.so' => ["%{build#{PREFIX}/lib/hybris/library/,stdlib}X.cc"] do |t|
   puts "@ Compiling #{t.source}"
   sh "mkdir -p #{File.dirname(t.name)}"
   sh "#{CXX} #{t.source} -o#{t.name} #{STDLIB_CFLAGS} #{STDLIB_LFLAGS}"
end



#----------------------------- Linking ----------------------------

file TARGET => OBJECTS[:BIN]  do
	puts "@ Linking #{TARGET}"
	sh "mkdir -p build#{PREFIX}/bin/"
	BIN_OBJECTS = (OBJECTS[:BIN].uniq - ['src/parser.cpp', 'src/lexer.cpp']).join(' ')
    sh "#{CXX} #{CXXFLAGS} -o build#{PREFIX}/bin/#{TARGET} #{BIN_OBJECTS} #{LDFLAGS}"
end

file "lib#{TARGET}.so.1.0" => OBJECTS[:LIB] do
	puts "@ Linking lib#{TARGET}.so.1.0"
	sh "mkdir -p build#{PREFIX}/lib/"
	LIB_OBJECTS = (OBJECTS[:LIB].uniq - ['src/parser.cpp', 'src/lexer.cpp']).join(' ')
	sh "#{CXX} -shared -Wl,-soname,lib#{TARGET}.so.1 -o build#{PREFIX}/lib/lib#{TARGET}.so.1.0 #{LIB_OBJECTS} #{STDLIB_LFLAGS}"
	sh "ln -sf lib#{TARGET}.so.1.0 build#{PREFIX}/lib/lib#{TARGET}.so"
	sh "ln -sf lib#{TARGET}.so.1.0 build#{PREFIX}/lib/lib#{TARGET}.so.1"	
end




