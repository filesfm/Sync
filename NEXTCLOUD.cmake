# keep the application name and short name the same or different for dev and prod build
# or some migration logic will behave differently for each build
if(NEXTCLOUD_DEV)
    set( APPLICATION_NAME       "Files.fm Sync 2025 Dev" )
    set( APPLICATION_SHORTNAME  "Files.fm Sync 2025 Dev" )
    set( APPLICATION_EXECUTABLE "files_fm_sync" )
    set( APPLICATION_ICON_NAME  "files_fm_sync" )
else()
    set( APPLICATION_NAME       "Files.fm Sync 2025" )
    set( APPLICATION_SHORTNAME  "Files.fm Sync 2025" )
    set( APPLICATION_EXECUTABLE "files_fm_sync" )
    set( APPLICATION_ICON_NAME  "files_fm_sync" )
endif()

set( APPLICATION_CONFIG_NAME "${APPLICATION_EXECUTABLE}" )
set( APPLICATION_DOMAIN     "files.fm" )
set( APPLICATION_VENDOR     "Files.fm SIA" )
set( APPLICATION_UPDATE_URL "https://files.fm/sync-share#setup" CACHE STRING "URL for updater" )
set( APPLICATION_HELP_URL   "" CACHE STRING "URL for the help menu" )

if(APPLE AND APPLICATION_NAME STREQUAL "Nextcloud" AND EXISTS "${CMAKE_SOURCE_DIR}/theme/colored/Nextcloud-macOS-icon.svg")
    set( APPLICATION_ICON_NAME "Nextcloud-macOS" )
    message("Using macOS-specific application icon: ${APPLICATION_ICON_NAME}")
endif()

set( APPLICATION_ICON_SET   "SVG" )
set( APPLICATION_SERVER_URL "https://webdav.files.fm" CACHE STRING "URL for the server to use. If entered, the UI field will be pre-filled with it" )
set( APPLICATION_SERVER_URL_ENFORCE OFF ) # If set and APPLICATION_SERVER_URL is defined, the server can only connect to the pre-defined URL
set( APPLICATION_REV_DOMAIN "com.files.fm.sync" )
set( APPLICATION_VIRTUALFILE_SUFFIX "filesfm" CACHE STRING "Virtual file suffix (not including the .)")
set( APPLICATION_OCSP_STAPLING_ENABLED OFF )
set( APPLICATION_FORBID_BAD_SSL OFF )

set( LINUX_PACKAGE_SHORTNAME "nextcloud" )
# The compiled app's own desktop-file/D-Bus ID -- kept equal to
# APPLICATION_REV_DOMAIN alone (com.files.fm.sync) rather than appending
# LINUX_PACKAGE_SHORTNAME, since that's the ID the Flatpak/Snap manifests
# use and Flatpak requires the exported desktop file to be named exactly
# <manifest-id>.desktop.
set( LINUX_APPLICATION_ID "${APPLICATION_REV_DOMAIN}")

set( THEME_CLASS            "NextcloudTheme" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
#set( CRASHREPORTER_SUBMIT_URL "https://crash-reports.owncloud.com/submit" CACHE STRING "URL for crash reporter" )
#set( CRASHREPORTER_ICON ":/owncloud-icon.png" )

## Updater options
option( BUILD_UPDATER "Build updater" ON )

option( WITH_PROVIDERS "Build with providers list" ON )

option( ENFORCE_VIRTUAL_FILES_SYNC_FOLDER "Enforce use of virtual files sync folder when available" OFF )
option( DISABLE_VIRTUAL_FILES_SYNC_FOLDER "Disable use of virtual files sync folder even when available" OFF )

option(ENFORCE_SINGLE_ACCOUNT "Enforce use of a single account in desktop client" OFF)

option( DO_NOT_USE_PROXY "Do not use system wide proxy, instead always do a direct connection to server" OFF )

## Theming options
set(NEXTCLOUD_BACKGROUND_COLOR "#242424" CACHE STRING "Default Nextcloud background color")
set( APPLICATION_WIZARD_HEADER_BACKGROUND_COLOR ${NEXTCLOUD_BACKGROUND_COLOR} CACHE STRING "Hex color of the wizard header background")
set( APPLICATION_WIZARD_HEADER_TITLE_COLOR "#ffffff" CACHE STRING "Hex color of the text in the wizard header")
option( APPLICATION_WIZARD_USE_CUSTOM_LOGO "Use the logo from ':/client/theme/colored/wizard_logo.(png|svg)' else the default application icon is used" ON )

#
## Windows Shell Extensions & MSI - IMPORTANT: Generate new GUIDs for custom builds with "guidgen" or "uuidgen"
#
if(WIN32)
    # Context Menu
    set( WIN_SHELLEXT_CONTEXT_MENU_GUID      "{A64B640C-9C3D-494F-934A-F5C5B528692E}" )

    # Overlays
    set( WIN_SHELLEXT_OVERLAY_GUID_ERROR     "{E0342B74-7593-4C70-9D61-22F294AAFE05}" )
    set( WIN_SHELLEXT_OVERLAY_GUID_OK        "{E1094E94-BE93-4EA2-9639-8475C68F3886}" )
    set( WIN_SHELLEXT_OVERLAY_GUID_OK_SHARED "{E243AD85-F71B-496B-B17E-B8091CBE93D2}" )
    set( WIN_SHELLEXT_OVERLAY_GUID_SYNC      "{E3D6DB20-1D83-4829-B5C9-941B31C0C35A}" )
    set( WIN_SHELLEXT_OVERLAY_GUID_WARNING   "{E4977F33-F93A-4A0A-9D3C-83DEA0EE8483}" )

    # MSI Upgrade Code (without brackets)
    set( WIN_MSI_UPGRADE_CODE                "DEC0DC8D-7D21-41B0-99C8-986CACB24484" )

    # Windows build options
    option( BUILD_WIN_MSI "Build MSI scripts and helper DLL" OFF )
    option( BUILD_WIN_TOOLS "Build Win32 migration tools" OFF )
endif()

if (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER_EQUAL 11.0)
    option( BUILD_FILE_PROVIDER_MODULE "Build the macOS virtual files File Provider module" OFF )
endif()
