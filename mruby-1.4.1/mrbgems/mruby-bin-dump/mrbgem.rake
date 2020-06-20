MRuby::Gem::Specification.new('mruby-bin-dump') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Cores Co., Ltd.'
  spec.summary = 'object dump'
  spec.bins = %w(mruby-dump)

  case spec.mruby.primary_toolchain.to_sym
  #when :gcc
  #  spec.linker.flags << "-Wl,-Map=#{build.build_dir}/bin/mruby-dump.map,--cref"
  when :visualcpp
    spec.linker.flags << "/MAP"
  end

  def map_to_syms(infile, outfile)
    istrm = File.open(infile, "r");
    ostrm = File.open(outfile, "w+");

    line = istrm.readline
    re = /^\s+Address\s+Publics by Value\s+Rva\+Base\s+Lib\:Object$/
    while !line.index(re)
      line = istrm.readline
    end

    while !istrm.eof
      line = istrm.readline

      newline = line.gsub(/\s+$/, "");
      re = /^\s*[0-9|a-f]+:[0-9|a-f]+\s+([0-9|a-z|A-Z|\?@_]+)\s+([0-9|a-f]+)\s+(f i|f  |   )\s+.+$/
      if !newline.index(re)
        next
      end

      sectype = newline.gsub(re, '\3');

      if (sectype == "f i") || (sectype == "f  ")
        newline = newline.gsub(re, '\2 T \1')
      #else if sectype == "rodata"
      #    newline = newline.gsub(re, '\2 R \1')
      else
        newline = newline.gsub(re, '\2 A \1')
      end

      ostrm.puts(newline)
    end
    istrm.close
    ostrm.close
  end

  spec.post_build_event = Proc.new do
    case current_target.primary_toolchain.to_sym
    when :gcc
      sh "nm -n #{build.build_dir}/bin/mruby-dump > #{build.build_dir}/bin/mruby-dump.syms"
    when :visualcpp
      map_to_syms "#{build.build_dir}/bin/mruby-dump.map", "#{build.build_dir}/bin/mruby-dump.syms"
    end
    sh "#{build.build_dir}/bin/mruby-dump #{build.build_dir}/src/preset_symbols.c"
  end
end
