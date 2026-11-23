# NetBSD image tests.
#
# check_netbsd contains three test cases, one per supported NetBSD image.
# A test case skips itself when its LIBVMI_NETBSD_IMAGE_* environment
# variable is not set, so the suite can run in CI without the image files.

add_executable(check_netbsd test_netbsd.c)
target_compile_options(check_netbsd PRIVATE ${Check_CFLAGS})
target_link_libraries(check_netbsd vmi_shared ${Check_LIBRARIES} ${GLIB_LIBRARIES})
set_target_properties(check_netbsd
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME test_netbsd_image_9_3
    COMMAND ${CMAKE_COMMAND} -E env
    LIBVMI_NETBSD_TEST_CASE=LIBVMI_NETBSD_IMAGE_9_3
    $<TARGET_FILE:check_netbsd>)
set_tests_properties(test_netbsd_image_9_3 PROPERTIES
    SKIP_RETURN_CODE 0)

add_test(NAME test_netbsd_image_10_1
    COMMAND ${CMAKE_COMMAND} -E env
    LIBVMI_NETBSD_TEST_CASE=LIBVMI_NETBSD_IMAGE_10_1
    $<TARGET_FILE:check_netbsd>)
set_tests_properties(test_netbsd_image_10_1 PROPERTIES
    SKIP_RETURN_CODE 0)

add_test(NAME test_netbsd_image_10_99
    COMMAND ${CMAKE_COMMAND} -E env
    LIBVMI_NETBSD_TEST_CASE=LIBVMI_NETBSD_IMAGE_10_99
    $<TARGET_FILE:check_netbsd>)
set_tests_properties(test_netbsd_image_10_99 PROPERTIES
    SKIP_RETURN_CODE 0)

# Offline validation of the shipped configs and symbol tables.
add_test(NAME test_netbsd_offline
    COMMAND ${CMAKE_COMMAND} -E env sh
    ${CMAKE_CURRENT_SOURCE_DIR}/run_netbsd_tests.sh)
