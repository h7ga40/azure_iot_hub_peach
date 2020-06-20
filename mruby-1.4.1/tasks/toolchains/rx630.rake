MRuby::Toolchain.new(:rx630) do |conf, _params|
  toolchain :gcc

  conf.cc do |cc|
    cc.command="rx-elf-gcc"
    cc.flags << "-Wall -Og -g2 -g -mlittle-endian-data -mcpu=rx600 -m64bit-doubles"
    cc.defines << %w(__RX_LITTLE_ENDIAN__)
  end

  conf.cxx do |cxx|
    cxx.command="rx-elf-g++"
    cxx.flags << "-Wall -Og -g2 -g -mlittle-endian-data -mcpu=rx600 -m64bit-doubles"
    cxx.defines << %w(__RX_LITTLE_ENDIAN__)
  end

  conf.linker do |linker|
    linker.command="rx-elf-ld"
  end

  conf.archiver do |archiver|
    archiver.command = "rx-elf-ar"
    archiver.archive_options = 'rcs %{outfile} %{objs}'
  end
end

