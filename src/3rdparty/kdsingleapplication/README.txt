This is the src directory of KDAB's KDSingleApplication library (v1.2.1),
vendored from https://github.com/KDAB/KDSingleApplication, MIT licensed.

Used on Linux only, replacing the qtsingleapplication/qtlockedfile solution
there, because it is built on Qt's own QLockFile, which correctly recovers
from a stale lock left behind by a process that died without cleaning up
(the old qtsingleapplication/QSharedMemory based mechanism does not, and can
get permanently stuck letting every later launch believe it is primary).
