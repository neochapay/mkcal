set(SRC_STORAGE
	tst_storage.cpp)
set(HEADERS_STORAGE
	tst_storage.h)

add_executable(tst_storage ${SRC_STORAGE} ${HEADERS_STORAGE})

target_include_directories(tst_storage PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(tst_storage
	Qt6::DBus
	Qt6::Test
	KF6::CalendarCore
	PkgConfig::SQLITE3
	PkgConfig::TIMED
	mkcal-qt6)

add_test(tst_storage tst_storage)
add_definitions(-DTIMED_SUPPORT)

set(SRC_PERF
	tst_perf.cpp)
set(HEADERS_PERF
	tst_perf.h)

add_executable(tst_perf ${SRC_PERF} ${HEADERS_PERF})

target_include_directories(tst_perf PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(tst_perf
	Qt6::Test
	KF6::CalendarCore
	mkcal-qt6)

add_test(tst_perf tst_perf)

add_executable(tst_load tst_load.cpp)

target_include_directories(tst_load PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(tst_load
	Qt6::Test
	KF6::CalendarCore
	mkcal-qt6)

add_test(tst_load tst_load)

if(INSTALL_TESTS)
	install(TARGETS tst_storage
		DESTINATION /opt/tests/mkcal)
	install(TARGETS tst_load
		DESTINATION /opt/tests/mkcal)
	install(TARGETS tst_perf
		DESTINATION /opt/tests/mkcal)
	install(FILES tests.xml
		DESTINATION /opt/tests/mkcal)
endif()
