llvm_map_components_to_libnames(llvm_libs
        bitwriter core
        ipo
        irreader
        instcombine
        instrumentation
        target
        linker
        analysis
        scalaropts
        support
        transformutils
        AggressiveInstCombine
        demangle
        mc)

add_executable( wpa wpa.cpp )

target_link_libraries( wpa aser ${llvm_libs} )

set_target_properties( wpa PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

