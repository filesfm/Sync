find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)


add_library(filesfm OBJECT
    messaging.cpp
    lockwatcher.cpp
    filehandles.cpp
)

set_target_properties(filesfm PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(filesfm PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    $<$<PLATFORM_ID:Windows>:ntdll>
)
