set(SRC
	extendedcalendar.cpp
	extendedstorage.cpp
	sqliteformat.cpp
	sqlitestorage.cpp
	servicehandler.cpp
        alarmhandler.cpp
	logging.cpp
	semaphore_p.cpp)
set(HEADERS
	extendedcalendar.h
	extendedstorage.h
	extendedstorageobserver.h
	sqlitestorage.h
	servicehandlerif.h
	servicehandler.h
	dummystorage.h
	mkcal_export.h
	invitationhandlerif.h)

set(PRIVATE_HEADERS
        alarmhandler_p.h
        logging_p.h
        semaphore_p.h
        sqliteformat.h
        )

add_library(mkcal-qt6 SHARED ${SRC} ${HEADERS} ${PRIVATE_HEADERS})

target_compile_definitions(mkcal-qt6 PRIVATE MKCAL_LIBRARY)
target_link_libraries(mkcal-qt6
		PRIVATE
        Qt6::DBus
	PkgConfig::SQLITE3
	PkgConfig::TIMED
		PUBLIC
        Qt6::Gui
        KF6::CalendarCore)

set_target_properties(mkcal-qt6 PROPERTIES
	SOVERSION ${PROJECT_VERSION_MAJOR}
	VERSION ${PROJECT_VERSION})

add_definitions(-fvisibility=hidden -fvisibility-inlines-hidden)
add_definitions(-DMKCALPLUGINDIR="${CMAKE_INSTALL_FULL_LIBDIR}/mkcalplugins")
add_definitions(-DTIMED_SUPPORT)

# Install the library
install(TARGETS mkcal-qt6
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install headers
install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mkcal-qt6
	COMPONENT Devel)

ecm_generate_pkgconfig_file(
        BASE_NAME libmkcal-qt6
        LIB_NAME mkcal-qt6
        INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/mkcal-qt6
	DESCRIPTION ${PROJECT_DESCRIPTION}
	INSTALL)
