cmake_minimum_required(VERSION 3.16)
project(EasyBC)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "/usr/bin/flex")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/preprocessing)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/branchNum)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/differentialCrypt)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/diffSingleKey)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/sbox)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/util)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/data)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/z3/include)


INCLUDE_DIRECTORIES(/usr/include/)
LINK_DIRECTORIES(/usr/lib)
LINK_DIRECTORIES(/home/xuws/.local/lib)   # 若上一步装到了 lib64，就写 lib64
INCLUDE_DIRECTORIES(/home/xuws/.local/include)

# s3lab
INCLUDE_DIRECTORIES(/home/xuws/gurobi1302/linux64/include)
LINK_DIRECTORIES(/home/xuws/gurobi1302/linux64/lib)

#add_subdirectory(lib)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -gdwarf-2")

FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(BISON REQUIRED)
message( ${CMAKE_CURRENT_SOURCE_DIR} )
BISON_TARGET(parser lib/language/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
)
FLEX_TARGET(lexer lib/language/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

ADD_EXECUTABLE(EasyBC
        ${FLEX_lexer_OUTPUTS}
        ${BISON_parser_OUTPUTS}
        main.cpp
        include/preprocessing/Value.h
        include/preprocessing/Interpreter.h
        include/preprocessing/ValueCommon.h
        include/preprocessing/ThreeAddressNode.h
        include/preprocessing/ProcedureH.h
        include/preprocessing/Transformer.h
        include/branchNum/BranchNum.h
        include/differential/DiffSBMILP.h
        include/differential/DiffSWMILP.h
        include/differential/DiffMILPcons.h
        include/division/Div2SetMILP.h
        include/division/Div3SetMILP.h
        include/division/BdptConfig.h
        include/division/BdptSolveResult.h
        include/division/DivMILPcons.h
        include/division/BdptMILPcons.h
        include/sbox/SboxModel.h
        include/sbox/SboxExtC.h
        include/sbox/Check.h
        include/sbox/Reduction.h
        include/sbox/superball.h
        include/sbox/superballMGR.h
        include/sbox/SyntaxGuided.h
        include/sbox/SboxDivTrails.h
        include/sbox/DivTrailsModel.h
        include/util/utilities.h
        lib/preprocessing/ASTNode.cpp
        lib/preprocessing/ValueCommon.cpp
        lib/preprocessing/ProcedureH.cpp
        lib/preprocessing/Transformer.cpp
        lib/branchNum/BranchNum.cpp
        lib/sbox/SboxModel.cpp
        lib/sbox/Check.cpp
        lib/sbox/Reduction.cpp
        lib/sbox/superball.cpp
        lib/sbox/superballMGR.cpp
        lib/sbox/SboxDivTrails.cpp
        lib/sbox/DivTrailsModel.cpp
        lib/util/utilities.cpp
        lib/division/DivMILPcons.cpp
        lib/division/BdptMILPcons.cpp
        lib/division/BdptConfig.cpp
        lib/division/Div2SetMILP.cpp
        lib/division/Div3SetMILP.cpp
        lib/differential/DiffSBMILP.cpp
        lib/differential/DiffSWMILP.cpp
        lib/differential/DiffMILPcons.cpp include/differential/DiffRBMILP.h lib/differential/DiffRBMILP.cpp include/linear/LinearMILPcons.h lib/linear/LinearMILPcons.cpp include/linear/LinearSBMILP.h lib/linear/LinearSBMILP.cpp include/differential/DiffRWMILP.h lib/differential/DiffRWMILP.cpp)


find_library(gurobi_static NAMES libgurobi_g++8.5.a gurobi_c++
        PATHS /home/xuws/gurobi1302/linux64/lib)

# 动态库名随大版本变化（12→libgurobi120.so，13→libgurobi130.so），自动探测
file(GLOB gurobi_dynamic
        /home/xuws/gurobi1302/linux64/lib/libgurobi[0-9][0-9][0-9].so
        /home/xuws/gurobi1302/linux64/lib/libgurobi[0-9][0-9].so)

if(NOT gurobi_static)
    message(FATAL_ERROR "Failed to find Gurobi static lib under ${GUROBI_HOME}/lib")
endif()
if(NOT gurobi_dynamic)
    message(FATAL_ERROR "Failed to find Gurobi shared lib (libgurobiXXX.so) under ${GUROBI_HOME}/lib")
endif()
message(STATUS "Gurobi static : ${gurobi_static}")
message(STATUS "Gurobi dynamic: ${gurobi_dynamic}")


TARGET_LINK_LIBRARIES(EasyBC ${FLEX_LIBRARIES} jsoncpp)
TARGET_LINK_LIBRARIES(EasyBC ${gurobi_static} ${gurobi_dynamic} ${PROJECT_SOURCE_DIR}/z3/bin/libz3.so)
