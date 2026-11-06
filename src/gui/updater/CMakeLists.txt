if(BUILD_OWNCLOUD_OSX_BUNDLE AND NOT BUILD_LIBRARIES_ONLY)
    if (SPARKLE_FOUND)
        # Updates are verified by the EdDSA SUPublicEDKey in Info.plist
        # (Sparkle 2 rejects code-signing-only updates); no legacy DSA key.
        install(FILES ${CMAKE_SOURCE_DIR}/admin/osx/deny_autoupdate_com.owncloud.desktopclient.plist
                DESTINATION ${OWNCLOUD_OSX_BUNDLE}/Contents/Resources)
    endif()
endif()

