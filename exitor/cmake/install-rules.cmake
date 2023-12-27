install(
        TARGETS EXitor
        RUNTIME #
        COMPONENT EXitor
)

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/assets
        DESTINATION /
)