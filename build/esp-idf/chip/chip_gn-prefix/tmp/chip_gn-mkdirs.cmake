# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/icewalker/esp-idf/esp-matter/connectedhomeip/connectedhomeip"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/tmp"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/src/chip_gn-stamp"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/src"
  "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/src/chip_gn-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/src/chip_gn-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/icewalker/esp-idf/esp-matter/examples/light_relay/build/esp-idf/chip/chip_gn-prefix/src/chip_gn-stamp${cfgdir}") # cfgdir has leading slash
endif()
