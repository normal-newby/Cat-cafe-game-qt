cmake_minimum_required(VERSION 3.21)
project(CatCafe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put the game and the test programs side by side in the build folder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia Test)

# Turns on Qt's code generators. The important one is "moc": it reads every header that
# contains Q_OBJECT and writes the extra C++ that makes signals and slots work.
qt_standard_project_setup()

add_compile_options(-Wall -Wextra)

# ---------------------------------------------------------------------------------------
# catcafe_core: all of the game's code except main(). It is a library so that both the
# game and the tests can link it.
# ---------------------------------------------------------------------------------------
qt_add_library(catcafe_core STATIC
    src/Assets.h                    src/Assets.cpp
    src/MainWindow.h                src/MainWindow.cpp
    src/audio/SoundManager.h        src/audio/SoundManager.cpp
    src/items/Item.h
    src/items/Ingredient.h          src/items/Ingredient.cpp
    src/items/Cup.h                 src/items/Cup.cpp
    src/items/ItemCatalog.h         src/items/ItemCatalog.cpp
    src/screens/MenuScreen.h        src/screens/MenuScreen.cpp
    src/screens/InstructionsScreen.h src/screens/InstructionsScreen.cpp
    src/ui/ImageButton.h            src/ui/ImageButton.cpp
    src/world/SpriteAnimation.h     src/world/SpriteAnimation.cpp
    src/world/Player.h              src/world/Player.cpp
    src/world/NonCollidable.h       src/world/NonCollidable.cpp
    src/world/Decoration.h
    src/world/Interactable.h        src/world/Interactable.cpp
    src/world/Counter.h             src/world/Counter.cpp
    src/world/PrepTable.h           src/world/PrepTable.cpp
    src/world/Trashbin.h            src/world/Trashbin.cpp
    src/world/Chair.h               src/world/Chair.cpp
    src/world/Customer.h            src/world/Customer.cpp
    src/world/SpawnOdds.h           src/world/SpawnOdds.cpp
    src/world/CafeScene.h           src/world/CafeScene.cpp
    src/screens/GameScreen.h        src/screens/GameScreen.cpp
    src/panels/HeldItemPanel.h      src/panels/HeldItemPanel.cpp
    src/panels/RestockingPanel.h    src/panels/RestockingPanel.cpp
    src/panels/OrderCard.h          src/panels/OrderCard.cpp
    src/panels/OrderPanel.h         src/panels/OrderPanel.cpp
    src/panels/UpgradesPanel.h      src/panels/UpgradesPanel.cpp
)
target_include_directories(catcafe_core PUBLIC src)
target_link_libraries(catcafe_core PUBLIC Qt6::Widgets Qt6::Multimedia)

# The Qt resource system: these files are compiled into the program and are then read
# through paths that start with ":/", for example ":/images/boba.png".
# The 13 MB background music is deliberately left out (see below).
file(GLOB ASSET_IMAGES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} assets/images/*.png)
qt_add_resources(catcafe_core "assets"
    PREFIX "/"
    BASE "assets"
    FILES
        ${ASSET_IMAGES}
        assets/sounds/click.wav
        assets/sounds/meowHappy.wav
        assets/sounds/meowAngry.wav
        assets/fonts/WaterlilyScript.ttf
)

# ---------------------------------------------------------------------------------------
# CatCafe: the game itself.
# ---------------------------------------------------------------------------------------
qt_add_executable(CatCafe src/main.cpp)
set_target_properties(CatCafe PROPERTIES WIN32_EXECUTABLE ON)   # no console window
target_link_libraries(CatCafe PRIVATE catcafe_core)

# The music is too big to compile in comfortably, so it is copied next to the .exe and
# loaded from disk when the game runs.
add_custom_command(TARGET CatCafe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:CatCafe>/assets/sounds
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/sounds/backgroundMusic.wav
            $<TARGET_FILE_DIR:CatCafe>/assets/sounds/backgroundMusic.wav
)

# ---------------------------------------------------------------------------------------
# Tests: one small program per file in tests/.
# ---------------------------------------------------------------------------------------
enable_testing()

function(catcafe_add_test name)
    qt_add_executable(tst_${name} tests/tst_${name}.cpp)
    target_link_libraries(tst_${name} PRIVATE catcafe_core Qt6::Test)
    add_dependencies(tst_${name} CatCafe)   # so the music has been copied before tests run
    add_test(NAME tst_${name} COMMAND tst_${name})
    # On Windows, Qt Test prints nothing unless it believes a console is attached. This
    # makes it always print, so a failing test shows its reason wherever it is run from.
    set_tests_properties(tst_${name} PROPERTIES ENVIRONMENT "QT_FORCE_STDERR_LOGGING=1")
endfunction()

catcafe_add_test(assets)
catcafe_add_test(items)
catcafe_add_test(widgets)
catcafe_add_test(screens)
catcafe_add_test(mainwindow)
catcafe_add_test(player)
catcafe_add_test(interactables)
catcafe_add_test(customers)
catcafe_add_test(cafescene)
catcafe_add_test(gamescreen)
catcafe_add_test(panels)
catcafe_add_test(sound)
