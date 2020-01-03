cmake_minimum_required( VERSION 3.16 )

project( metamorf )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ccf" ${CMAKE_MODULE_PATH} )
include(ccf_init)

ccf_3p(spdlog TAG "v1.4.2")
ccf_3p(magic_enum TAG "v0.6.3")
ccf_3p(hana DEFAULT)
ccf_3p(debug_assert TAG "v1.3.3")
ccf_3p(span DEFAULT)

ccf_add_all(TARGET)

ccf_dep_resolve()
