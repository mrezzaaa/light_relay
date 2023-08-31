# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/icewalker/esp-idf/esp-idf/components/bootloader/subproject"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/tmp"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/src"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
