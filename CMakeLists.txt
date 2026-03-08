cmake_minimum_required(VERSION 3.16)
project(CMatrixPolymorphic C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(cmatrix
        .c/complex.c
        .c/fieldinfo.c
        .c/matrix.c
)

target_include_directories(cmatrix PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/.h
)

add_executable(app .c/main.c)
target_link_libraries(app PRIVATE cmatrix)

enable_testing()
add_executable(tests tests.c)
target_link_libraries(tests PRIVATE cmatrix)
add_test(NAME cmatrix_tests COMMAND tests)

add_executable(formula_tests formula_tests.c)
target_link_libraries(formula_tests PRIVATE cmatrix)
add_test(NAME cmatrix_formula_tests COMMAND formula_tests)

add_custom_target(run_formula_tests
        COMMAND formula_tests
        DEPENDS formula_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
