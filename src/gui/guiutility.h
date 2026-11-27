/*
 * Copyright (C) by Christian Kamm <mail@ckamm.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#ifndef GUIUTILITY_H
#define GUIUTILITY_H

#include <QString>
#include <QUrl>
#include <QWidget>

#include "common/pinstate.h"

namespace OCC {
namespace Utility {

    /** RAII guard that scrubs LD_LIBRARY_PATH/QT_PLUGIN_PATH/QML2_IMPORT_PATH for its lifetime on Linux.
     *
     * Wrap any QProcess start or fork+exec of a host executable (file manager, xdg-open, ...) in
     * one of these: an AppImage build needs those vars for itself, but a host process that
     * inherits them tries to load our bundled libs instead of its own and fails with "symbol
     * lookup error". Restores the vars on scope exit. Thread-safe (serializes via a shared
     * mutex), since mutating process environment variables concurrently is not otherwise safe.
     */
    class ScopedHostEnvironment
    {
    public:
        ScopedHostEnvironment();
        ~ScopedHostEnvironment();

        ScopedHostEnvironment(const ScopedHostEnvironment &) = delete;
        ScopedHostEnvironment &operator=(const ScopedHostEnvironment &) = delete;

    private:
        class Private;
        Private *_d;
    };

    /** Open a url or local path with the user's preferred handler (browser, file manager, ...).
     *
     * Drop-in replacement for QDesktopServices::openUrl(): uses ScopedHostEnvironment so Linux
     * AppImage builds don't break the opener (see its documentation).
     */
    bool openUrlSafely(const QUrl &url);

    /** Open an url in the browser.
     *
     * If launching the browser fails, display a message.
     */
    bool openBrowser(const QUrl &url, QWidget *errorWidgetParent = nullptr);

    /** Start composing a new email message.
     *
     * If launching the email program fails, display a message.
     */
    bool openEmailComposer(const QString &subject, const QString &body,
        QWidget *errorWidgetParent);

    /** Returns a translated string indicating the current availability.
     *
     * This will be used in context menus to describe the current state.
     */
    QString vfsCurrentAvailabilityText(VfsItemAvailability availability);

    /** Translated text for "making items always available locally" */
    QString vfsPinActionText();

    /** Translated text for "free up local space" (and unpinning the item) */
    QString vfsFreeSpaceActionText();

} // namespace Utility
} // namespace OCC

#endif
