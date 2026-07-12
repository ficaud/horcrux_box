# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wifi_apsta_mode)

target_sources(app PRIVATE
    src/main.c
    src/access-point/src/dns.c
    src/access-point/src/http_server.c
    src/access-point/src/router.c
    src/access-point/src/handlers.c
    src/access-point/src/dhcp.c
    src/access-point/src/wifi_mgr.c
)

# Auto-generate page_captive.h from assets (HTML + CSS)
set(ASSETS_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/src/access-point/assets)
set(ASSETS_OUTPUT ${ASSETS_DIR}/page_captive.h)

add_custom_command(
    OUTPUT  ${ASSETS_OUTPUT}
    COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed-assets.py
            ${ASSETS_DIR}
            ${ASSETS_OUTPUT}
    DEPENDS ${ASSETS_DIR}/index.html
            ${ASSETS_DIR}/style.css
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed-assets.py
    COMMENT "Generating page_captive.h from assets..."
)

target_sources(app PRIVATE ${ASSETS_OUTPUT})

target_include_directories(app PRIVATE
    src/access-point/inc
    src/access-point/assets
)
