require 'forwardable'
require 'digest/md5'

module MRuby
  class Command
    include Rake::DSL
    extend Forwardable
    def_delegators :@build, :filename, :objfile, :libfile, :exefile, :cygwin_filename
    attr_accessor :build, :command

    def initialize(build)
      @build = build
    end

    # clone is deep clone without @build
    def clone
      target = super
      excepts = %w(@build)
      instance_variables.each do |attr|
        unless excepts.include?(attr.to_s)
          val = Marshal::load(Marshal.dump(instance_variable_get(attr))) # deep clone
          target.instance_variable_set(attr, val)
        end
      end
      target
    end

    NotFoundCommands = {}

    private
    def _run(options, params={})
      return sh command + ' ' + ( options % params ) if NotFoundCommands.key? @command
      begin
        sh build.filename(command) + ' ' + ( options % params )
      rescue RuntimeError
        NotFoundCommands[@command] = true
        _run options, params
      end
    end
  end

  class Command::Compiler < Command
    attr_accessor :flags, :include_paths, :defines, :source_exts
    attr_accessor :compile_options, :option_define, :option_include_path, :out_ext
    attr_accessor :cxx_compile_flag, :cxx_exception_flag

    def initialize(build, source_exts=[])
      super(build)
      @command = ENV['CC'] || 'cc'
      @flags = [ENV['CFLAGS'] || []]
      @source_exts = source_exts
      @include_paths = ["#{MRUBY_ROOT}/include"]
      @defines = %w()
      @option_include_path = '-I%s'
      @option_define = '-D%s'
      @compile_options = '%{flags} -o %{outfile} -c %{infile}'
    end

    alias header_search_paths include_paths
    def search_header_path(name)
      header_search_paths.find do |v|
        File.exist? build.filename("#{v}/#{name}").sub(/^"(.*)"$/, '\1')
      end
    end

    def search_header(name)
      path = search_header_path name
      path && build.filename("#{path}/#{name}").sub(/^"(.*)"$/, '\1')
    end

    def all_flags(_defineds=[], _include_paths=[], _flags=[], params={})
      define_flags = [defines, _defineds].flatten.map{ |d| option_define % d }
      include_path_flags = [include_paths, _include_paths].flatten.map do |f|
        if MRUBY_BUILD_HOST_IS_CYGWIN
          option_include_path % cygwin_filename(f)
        else
          option_include_path % filename(f)
        end
      end
      if params.length > 0
        __flags = flags.collect { |flag|
          if flag.is_a?(Array)
            flag.collect { |f|
              if f.include?('%')
                f % params
              else
                f
              end
              }.flatten
          else
            if flag.include?('%')
              flag % params
            else
              flag
            end
          end
        }.flatten
      else
        __flags = flags
      end
      [__flags, define_flags, include_path_flags, _flags].flatten.join(' ')
    end

    def run(outfile, infile, _defineds=[], _include_paths=[], _flags=[])
      FileUtils.mkdir_p File.dirname(outfile)
      _pp "CC", infile.relative_path, outfile.relative_path
      params = { :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      if MRUBY_BUILD_HOST_IS_CYGWIN
        _run compile_options, { :flags => all_flags(_defineds, _include_paths, _flags, params),
                                :infile => cygwin_filename(infile), :outfile => cygwin_filename(outfile),
                                :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      else
        _run compile_options, { :flags => all_flags(_defineds, _include_paths, _flags, params),
                                :infile => filename(infile), :outfile => filename(outfile),
                                :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      end
    end

    def define_rules(build_dir, source_dir='')
      @out_ext = build.exts.object
      gemrake = File.join(source_dir, "mrbgem.rake")
      rakedep = File.exist?(gemrake) ? [ gemrake ] : []

      if build_dir.include? "mrbgems/"
        generated_file_matcher = Regexp.new("^#{Regexp.escape build_dir}/(.*)#{Regexp.escape out_ext}$")
      else
        generated_file_matcher = Regexp.new("^#{Regexp.escape build_dir}/(?!mrbgems/.+/)(.*)#{Regexp.escape out_ext}$")
      end
      source_exts.each do |ext, compile|
        rule generated_file_matcher => [
          proc { |file|
            file.sub(generated_file_matcher, "#{source_dir}/\\1#{ext}")
          },
          proc { |file|
            get_dependencies(file) + rakedep
          }
        ] do |t|
          run t.name, t.prerequisites.first
        end

        rule generated_file_matcher => [
          proc { |file|
            file.sub(generated_file_matcher, "#{build_dir}/\\1#{ext}")
          },
          proc { |file|
            get_dependencies(file) + rakedep
          }
        ] do |t|
          run t.name, t.prerequisites.first
        end
      end
    end

    private
    def get_dependencies(file)
      file = file.ext('d') unless File.extname(file) == '.d'
      if File.exist?(file)
        deps = File.read(file).gsub("\\\n ", "").scan(/^\S+:\s+(.+)$/).flatten.map {|s| s.split(/(?<!\\) /) }.flatten
        deps.collect{ |f| f.gsub(/(\\ )/, " ") }.flatten
      else
        []
      end + [ MRUBY_CONFIG ]
    end
  end

  class Command::Linker < Command
    attr_accessor :flags, :library_paths, :flags_before_libraries, :libraries, :flags_after_libraries
    attr_accessor :link_options, :option_library, :option_library_path

    def initialize(build)
      super
      @build = build
      @command = ENV['LD'] || 'ld'
      @flags = (ENV['LDFLAGS'] || [])
      @flags_before_libraries, @flags_after_libraries = [], []
      @libraries = []
      @library_paths = []
      @option_library = '-l%s'
      @option_library_path = '-L%s'
      @link_options = "%{flags} -o %{outfile} %{objs} %{flags_before_libraries} %{libs} %{flags_after_libraries}"
    end

    def all_flags(_library_paths=[], _flags=[], params={})
      library_path_flags = [library_paths, _library_paths].flatten.map do |f|
        if MRUBY_BUILD_HOST_IS_CYGWIN
          option_library_path % cygwin_filename(f)
        else
          option_library_path % filename(f)
        end
      end
      if params.length > 0
        __flags = flags.collect { |flag|
          if flag.is_a?(Array)
            flag.collect { |f|
              if f.include?('%')
                f % params
              else
                f
              end
              }.flatten
          else
            if flag.include?('%')
              flag % params
            else
              flag
            end
          end
        }.flatten
      else
        __flags = flags
      end
      [__flags, library_path_flags, _flags].flatten.join(' ')
    end

    def library_flags(_libraries)
      [libraries, _libraries].flatten.map{ |d| option_library % d }.join(' ')
    end

    def run(outfile, objfiles, _libraries=[], _library_paths=[], _flags=[], _flags_before_libraries=[], _flags_after_libraries=[])
      FileUtils.mkdir_p File.dirname(outfile)
      library_flags = [libraries, _libraries].flatten.map { |d| option_library % d }

      _pp "LD", outfile.relative_path
      params = { :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      if MRUBY_BUILD_HOST_IS_CYGWIN
        _run link_options, { :flags => all_flags(_library_paths, _flags, params),
                             :outfile => cygwin_filename(outfile) , :objs => cygwin_filename(objfiles).join(' '),
                             :flags_before_libraries => [flags_before_libraries, _flags_before_libraries].flatten.join(' '),
                             :flags_after_libraries => [flags_after_libraries, _flags_after_libraries].flatten.join(' '),
                             :libs => library_flags.join(' '),
                             :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      else
        _run link_options, { :flags => all_flags(_library_paths, _flags, params),
                             :outfile => filename(outfile) , :objs => filename(objfiles).join(' '),
                             :flags_before_libraries => [flags_before_libraries, _flags_before_libraries].flatten.join(' '),
                             :flags_after_libraries => [flags_after_libraries, _flags_after_libraries].flatten.join(' '),
                             :libs => library_flags.join(' '),
                             :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      end
    end
  end

  class Command::Archiver < Command
    attr_accessor :archive_options

    def initialize(build)
      super
      @command = ENV['AR'] || 'ar'
      @archive_options = 'rs %{outfile} %{objs}'
    end

    def run(outfile, objfiles)
      FileUtils.mkdir_p File.dirname(outfile)
      _pp "AR", outfile.relative_path

      # reference from emar.py
      outfilebase = File.dirname(outfile)
      to_delete = []
      newargs = []
      objfiles.each do |orig_name|
        dir_name = File.dirname(orig_name)
        dir_name = dir_name.relative_path_from(Dir.pwd)
        base_name = File.basename(orig_name)
        parts = base_name.split('.')
        # h = Digest::MD5.new.update(orig_name).to_s
        h = Digest::MD5.new.update(orig_name).to_s.slice(0,4)
        parts[0] += '_' + h
        newname = parts.join('.')
        full_newname = File.join(dir_name, newname)
        if not File.exists?(full_newname)
          begin # it is ok to fail here, we just don't get hashing
            FileUtils.cp(orig_name, full_newname)
            newargs << full_newname
            to_delete << full_newname
          rescue
          end
        end
      end

      if MRUBY_BUILD_HOST_IS_CYGWIN
        _run archive_options, { :outfile => cygwin_filename(outfile), :objs => cygwin_filename(newargs).join(' '), :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      else
        _run archive_options, { :outfile => filename(outfile), :objs => filename(newargs).join(' '), :outdir => File.dirname(outfile).gsub('/', @build.file_separator), :outfilebase => File.basename(outfile, ".*") }
      end

      to_delete.each do |d|
        FileUtils.rm(d)
      end
    end
  end

  class Command::Yacc < Command
    attr_accessor :compile_options

    def initialize(build)
      super
      @command = 'bison'
      @compile_options = '-o %{outfile} %{infile}'
    end

    def run(outfile, infile)
      FileUtils.mkdir_p File.dirname(outfile)
      _pp "YACC", infile.relative_path, outfile.relative_path
      _run compile_options, { :outfile => filename(outfile) , :infile => filename(infile) }
    end
  end

  class Command::Gperf < Command
    attr_accessor :compile_options

    def initialize(build)
      super
      @command = 'gperf'
      @compile_options = '-L ANSI-C -C -p -j1 -i 1 -g -o -t -N mrb_reserved_word -k"1,3,$" %{infile} > %{outfile}'
    end

    def run(outfile, infile)
      FileUtils.mkdir_p File.dirname(outfile)
      _pp "GPERF", infile.relative_path, outfile.relative_path
      _run compile_options, { :outfile => filename(outfile) , :infile => filename(infile) }
    end
  end

  class Command::Git < Command
    attr_accessor :flags
    attr_accessor :clone_options, :pull_options, :checkout_options

    def initialize(build)
      super
      @command = 'git'
      @flags = %w[]
      @clone_options = "clone %{flags} %{url} %{dir}"
      @pull_options = "pull"
      @checkout_options = "checkout %{checksum_hash}"
    end

    def run_clone(dir, url, _flags = [])
      _pp "GIT", url, dir.relative_path
      _run clone_options, { :flags => [flags, _flags].flatten.join(' '), :url => url, :dir => filename(dir) }
    end

    def run_pull(dir, url)
      root = Dir.pwd
      Dir.chdir dir
      _pp "GIT PULL", url, dir.relative_path
      _run pull_options
      Dir.chdir root
    end

    def run_checkout(dir, checksum_hash)
      root = Dir.pwd
      Dir.chdir dir
      _pp "GIT CHECKOUT", checksum_hash
      _run checkout_options, { :checksum_hash => checksum_hash }
      Dir.chdir root
    end
  end

  class Command::Mrbc < Command
    attr_accessor :compile_options

    def initialize(build)
      super
      @command = nil
      @compile_options = "-B%{funcname} -o-"
    end

    def run(out, infiles, funcname)
      @command ||= @build.mrbcfile
      infiles = [infiles].flatten
      infiles.each do |f|
        _pp "MRBC", f.relative_path, nil, :indent => 2
      end
      IO.popen("#{filename @command} #{@compile_options % {:funcname => funcname}} #{filename(infiles).join(' ')}", 'r+') do |io|
        out.puts io.read
      end
      # if mrbc execution fail, drop the file
      if $?.exitstatus != 0
        File.delete(out.path)
        exit(-1)
      end
    end
  end

  class Command::CrossTestRunner < Command
    attr_accessor :runner_options
    attr_accessor :verbose_flag
    attr_accessor :flags

    def initialize(build)
      super
      @command = nil
      @runner_options = '%{flags} %{infile}'
      @verbose_flag = ''
      @flags = []
    end

    def run(testbinfile)
      puts "TEST for " + @build.name
      _run runner_options, { :flags => [flags, verbose_flag].flatten.join(' '), :infile => testbinfile }
    end
  end

end
