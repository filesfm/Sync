/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <kdsingleapplication.h>

#include <QString>

namespace OCC::Utility {

[[nodiscard]] inline KDSingleApplication makeLinuxSingleApplication(const QString &name)
{
    return KDSingleApplication(name, KDSingleApplication::Option::IncludeUsernameInSocketName);
}

}
