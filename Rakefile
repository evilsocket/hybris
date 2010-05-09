require 'rake'
require 'rake/clean'
require 'mkmf'

verbose(false)

TARGET  	  = 'hybris'
RELEASE 	  = '1.0b2'

FLEX		  = 'flex'
BISON		  = 'bison'
CXX           = 'g++'
WFLAGS        = '-w'
OPTIMIZATION  = '-O3 -pipe -fomit-frame-pointer -ffast-math'
CXXFLAGS      = "-Iinclude/ #{WFLAGS} #{OPTIMIZATION}" 
LDFLAGS       = '-ldl -lpcre -lpthread'
LIBXML_CFLAGS = "`xml2-config --cflags`"
LIBXML_LFLAGS = "`xml2-config --libs`"
LIBFFI_CFLAGS = "`pkg-config libffi --cflags`"
LIBFFI_LFLAGS = "`pkg-config libffi --libs`"
STDLIB_LFLAGS = "-ldl -lpcre -lcurl -lpthread #{LIBXML_LFLAGS} #{LIBFFI_LFLAGS}" 
STDLIB_CFLAGS = "#{WFLAGS} -L. -I./include/ #{OPTIMIZATION} -fPIC #{LIBXML_CFLAGS} #{LIBFFI_CFLAGS} -lhybris -lc -shared"
PREFIX        = "/usr"

CLEAN.include( 'src/**/*.o', 
               'src/**/*.lo', 
               TARGET, 
			   'include/config.h',
               'src/lexer.cpp', 'src/parser.cpp', 'src/parser.hpp', 
               'libhybris.so', 'libhybris.so.1', 'libhybris.so.1.0',
			   'build' )

#----------------------------- Files ----------------------------

SRC = FileList['src/**/*.cpp'] - ['src/lexer.l.cpp', 'src/parser.y.cpp'] + ['src/lexer.cpp', 'src/parser.cpp']

SOURCES = {
    :BIN => SRC,
    :LIB => SRC - ['src/main.cpp'],
    :STD => FileList['stdlib/**/*.cc']
}

OBJECTS = {
	:BIN => ['src/lexer.cpp', 'src/parser.cpp'] + SOURCES[:BIN].ext('o'),
	:LIB => ['src/lexer.cpp', 'src/parser.cpp'] + SOURCES[:LIB].ext('lo'),
	:STD => SOURCES[:STD].ext('so')
}

#----------------------------- Tasks ----------------------------

task :interpreter => TARGET
task :library     => "lib#{TARGET}.so.1.0"
task :modules     => OBJECTS[:STD]

task :exec    => [ :checkdeps, :config_h, :interpreter, :library ]
task :all     => [ :exec, :modules ]
task :debug do
	CXXFLAGS = "-Iinclude/ #{WFLAGS} -g -pg" 
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

task :install do
	raise '[ERROR] Must run as root' unless Process.uid == 0

	sh "install -m 0755 #{TARGET} #{PREFIX}/bin/"
	sh "mkdir -p #{PREFIX}/include/#{TARGET}"
	sh "cp include/*.h #{PREFIX}/include/#{TARGET}/"
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}"
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}/include"
	sh "mkdir -p #{PREFIX}/lib/#{TARGET}/library"
	sh "chmod -R 777 #{PREFIX}/lib/#{TARGET}/"
	sh "install -m 0644 lib#{TARGET}.so.1.0 #{PREFIX}/lib"
	sh "ln -sf #{PREFIX}/lib/lib#{TARGET}.so.1.0 #{PREFIX}/lib/lib#{TARGET}.so"
	sh "ln -sf #{PREFIX}/lib/lib#{TARGET}.so.1.0 #{PREFIX}/lib/lib#{TARGET}.so.1"
	sh "ldconfig"
	sh "cp -rf build/* #{PREFIX}/lib/hybris/library/"
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
    sh "#{BISON} -y -d -o#{t.name} #{t.source}"
end

rule '.lo' => '.cpp' do |t|
	puts "@ Compiling #{t.source}"
    sh "#{CXX} #{CXXFLAGS} -fPIC -o #{t.name} -c #{t.source}"
end

rule '.o' => '.cpp' do |t|
	puts "@ Compiling #{t.source}"
    sh "#{CXX} #{CXXFLAGS} -o #{t.name} -c #{t.source}"
end

rule '.so' => '.cc' do |t|
	puts "@ Compiling #{t.source}"
	output = t.source.ext("so")
	output['stdlib'] = 'build'
	sh "mkdir -p #{File.dirname(output)}"
	sh "#{CXX} #{t.source} -o#{output} #{STDLIB_CFLAGS} #{STDLIB_LFLAGS}"
end

#----------------------------- Linking ----------------------------

file TARGET => OBJECTS[:BIN]  do
	puts "@ Linking #{TARGET}"
	BIN_OBJECTS = (OBJECTS[:BIN].uniq - ['src/lexer.cpp', 'src/parser.cpp']).join(' ')
    sh "#{CXX} #{CXXFLAGS} -o #{TARGET} #{BIN_OBJECTS} #{LDFLAGS}"
end

file "lib#{TARGET}.so.1.0" => OBJECTS[:LIB] do
	puts "@ Linking lib#{TARGET}.so.1.0"
	LIB_OBJECTS = (OBJECTS[:LIB].uniq - ['src/lexer.cpp', 'src/parser.cpp']).join(' ')
	sh "#{CXX} -shared -Wl,-soname,lib#{TARGET}.so.1 -o lib#{TARGET}.so.1.0 #{LIB_OBJECTS} #{STDLIB_LFLAGS}"
	sh "ln -sf lib#{TARGET}.so.1.0 lib#{TARGET}.so"
	sh "ln -sf lib#{TARGET}.so.1.0 lib#{TARGET}.so.1"	
end




