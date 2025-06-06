FILE( GLOB_RECURSE Guayadeque_SOURCES_CPP "*.cpp" )
FILE( GLOB_RECURSE Guayadeque_SOURCES_C "*.c" )
FILE( GLOB_RECURSE Guayadeque_HEADERS "*.h" )


SET( Guayadeque_INCLUDE_DIRS "" )
FOREACH( _headerFile ${Guayadeque_HEADERS} )
    GET_FILENAME_COMPONENT( _dir ${_headerFile} PATH )
    LIST( APPEND Guayadeque_INCLUDE_DIRS ${_dir} )
ENDFOREACH()
LIST( REMOVE_DUPLICATES Guayadeque_INCLUDE_DIRS )

INCLUDE_DIRECTORIES( ${Guayadeque_INCLUDE_DIRS} )

ADD_EXECUTABLE( guayadeque ${Guayadeque_SOURCES_CPP} ${Guayadeque_SOURCES_C} ${Guayadeque_HEADERS} )

TARGET_LINK_LIBRARIES( guayadeque
        ${wxWidgets_LIBRARIES}
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_TAG_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        ${LIBCURL_LIBRARIES}
        ${LIBTAG_LIBRARIES}
        ${LIBDBUS_LIBRARIES}
        ${LIBGDKPIXBUF20_LIBRARIES}
        ${LIBGIO_LIBRARIES}
        ${LIBGPOD_LIBRARIES}
        ${LIBWXSQLITE3_LIBRARIES}
        ${LIBWXSQLITE33_LIBRARIES}
        ${LIBJSONCPP_LIBRARIES}
        ${ICU_LIBRARIES}
)

INSTALL( TARGETS guayadeque
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
