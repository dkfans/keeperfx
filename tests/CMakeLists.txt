add_subdirectory(doctest)

add_executable(test_rtree
	test_rtree.cpp
)
add_executable(THST::test_rtree ALIAS test_rtree)

target_link_libraries(test_rtree
	PRIVATE
		doctest::doctest
		THST
		-static stdc++ winpthread -dynamic
)

target_compile_definitions(test_rtree
	PRIVATE
		DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
)

add_executable(test_CreatureTree
	test_CreatureTree.cpp
	../src/creature/CreatureTree.cpp
)

target_include_directories(test_CreatureTree PUBLIC "../src")
target_link_libraries(test_CreatureTree
	PRIVATE
		doctest::doctest
		THST
		-static stdc++ winpthread -dynamic
)

target_compile_definitions(test_CreatureTree
	PRIVATE
		DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
)
