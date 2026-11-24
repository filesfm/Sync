#include "filehandles.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QtSystemDetection>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#elif defined(Q_OS_LINUX)
#include <climits>
#include <unistd.h>
#elif defined(Q_OS_MACOS)
#include <libproc.h>
#include <sys/proc_info.h>
#include <unistd.h>
#endif

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

// If your SDK doesn’t define it
#ifndef SystemHandleInformation
#define SystemHandleInformation 16
#endif

#ifndef ObjectNameInformation
#define ObjectNameInformation 1
#endif

#ifndef ObjectTypeInformation
#define ObjectTypeInformation 2
#endif

// Minimal OBJECT_TYPE_INFORMATION struct
#ifdef Q_OS_WIN
typedef struct _MY_OBJECT_TYPE_INFORMATION {
    UNICODE_STRING TypeName;
} MY_OBJECT_TYPE_INFORMATION, *PMY_OBJECT_TYPE_INFORMATION;

// OBJECT_NAME_INFORMATION
typedef struct _MY_OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name;
} MY_OBJECT_NAME_INFORMATION, *PMY_OBJECT_NAME_INFORMATION;

// SYSTEM_HANDLE_INFORMATION
typedef struct _SYSTEM_HANDLE {
    ULONG       ProcessId;
    BYTE        ObjectTypeNumber;
    BYTE        Flags;
    USHORT      Handle;
    PVOID       Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG HandleCount;
    SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;
#endif

// Function pointer types for dynamic loading
#ifdef Q_OS_WIN
using fNtQuerySystemInformation = NTSTATUS(NTAPI*)(
    SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

using fNtQueryObject = NTSTATUS(NTAPI*)(
    HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);
#endif

// Helpers
#ifdef Q_OS_WIN
static std::wstring getObjectTypeName(HANDLE handle, fNtQueryObject NtQueryObjectPtr) {
    BYTE buffer[1024];
    ULONG len = 0;

    NTSTATUS status = NtQueryObjectPtr(
        handle,
        static_cast<OBJECT_INFORMATION_CLASS>(ObjectTypeInformation),
        buffer,
        sizeof(buffer),
        &len
    );

    if (status == 0) {
        auto* oti = reinterpret_cast<PMY_OBJECT_TYPE_INFORMATION>(buffer);
        return std::wstring(oti->TypeName.Buffer, oti->TypeName.Length / sizeof(WCHAR));
    }
    return L"";
}
#endif

#ifdef Q_OS_WIN
static std::wstring getObjectName(HANDLE handle) {
    std::vector<WCHAR> path(MAX_PATH);
    // VOLUME_NAME_DOS (0x0) is needed to get the DOS path, e.g. C:\...
    DWORD path_len = GetFinalPathNameByHandleW(handle, path.data(), path.size(), VOLUME_NAME_DOS);
    if (path_len > path.size()) {
        path.resize(path_len);
        path_len = GetFinalPathNameByHandleW(handle, path.data(), path.size(), VOLUME_NAME_DOS);
    }

    if (path_len > 0 && path_len < path.size()) {
        std::wstring result(path.data(), path_len);
        if (result.rfind(L"\\\\?\\", 0) == 0) {
            return result.substr(4);
        }
        return result;
    }

    return L"";
}
#endif

QSet<QString> getFileHandles(const QString &path) {
#ifdef Q_OS_WIN
    QSet<QString> openFiles;
    if (path.isEmpty()) {
        return openFiles;
    }
    const std::wstring pathToWatch = path.toStdWString();
    // Dynamically load NT API functions
    static fNtQuerySystemInformation NtQuerySystemInformationPtr =
        (fNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");
    static fNtQueryObject NtQueryObjectPtr =
        (fNtQueryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryObject");

    if (!NtQuerySystemInformationPtr || !NtQueryObjectPtr) {
        qWarning() << "Could not load NtQuerySystemInformation / NtQueryObject";
        return openFiles;
    }

    ULONG size = 0x400000;
    std::vector<BYTE> buffer(size);
    ULONG needed = 0;
    NTSTATUS status;

    // Query all handles
    while ((status = NtQuerySystemInformationPtr(
                static_cast<SYSTEM_INFORMATION_CLASS>(SystemHandleInformation),
                buffer.data(),
                static_cast<ULONG>(buffer.size()),
                &needed)) == STATUS_INFO_LENGTH_MISMATCH) {
        buffer.resize(needed > buffer.size() ? needed : buffer.size() * 2);
    }
    if (!NT_SUCCESS(status)) {
        std::cerr << "NtQuerySystemInformation failed: " << std::hex << status << std::endl;
        return openFiles;
    }

    auto* info = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(buffer.data());
    ULONG handleCount = info->HandleCount;

    std::unordered_map<ULONG, HANDLE> processCache;
    std::unordered_map<BYTE, std::wstring> typeCache;
    const DWORD currentPid = GetCurrentProcessId();

    for (ULONG i = 0; i < handleCount; i++) {
        const SYSTEM_HANDLE& h = info->Handles[i];

        if (h.ProcessId == currentPid || h.ProcessId == 4) {
            continue;
        }

        // Open process (cache per PID)
        HANDLE processHandle = nullptr;
        if (processCache.count(h.ProcessId)) {
            processHandle = processCache[h.ProcessId];
        } else {
            processHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, h.ProcessId);
            processCache[h.ProcessId] = processHandle;
        }
        if (!processHandle) continue;

        // Duplicate handle into this process
        HANDLE dupHandle = nullptr;
        if (!DuplicateHandle(processHandle,
                             reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(h.Handle)),
                             GetCurrentProcess(),
                             &dupHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            continue;
        }

        // Determine object type
        std::wstring typeName;
        if (typeCache.count(h.ObjectTypeNumber)) {
            typeName = typeCache[h.ObjectTypeNumber];
        } else {
            typeName = getObjectTypeName(dupHandle, NtQueryObjectPtr);
            typeCache[h.ObjectTypeNumber] = typeName;
        }

        if (typeName == L"File") {
            if (GetFileType(dupHandle) == FILE_TYPE_DISK) {
                BY_HANDLE_FILE_INFORMATION fileInfo;
                if (GetFileInformationByHandle(dupHandle, &fileInfo)) {
                    if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        QString name = QDir::fromNativeSeparators(QString::fromWCharArray(getObjectName(dupHandle).c_str()));
                        if (!name.isEmpty() && name.indexOf(pathToWatch) == 0) {
                            openFiles.insert(name);
                        }
                    }
                }
            }
        }

        CloseHandle(dupHandle);
    }

    for (auto& kv : processCache) {
        if (kv.second) CloseHandle(kv.second);
    }
    return openFiles;
#elif defined(Q_OS_LINUX)
    QSet<QString> openFiles;
    if (path.isEmpty()) {
        return openFiles;
    }

    const pid_t currentPid = getpid();

    QDir procDir(QStringLiteral("/proc"));
    const auto pidEntries = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &pidStr : pidEntries) {
        bool pidIsNumeric = false;
        const qint64 pid = pidStr.toLongLong(&pidIsNumeric);
        if (!pidIsNumeric || pid == currentPid) {
            continue;
        }

        QDir fdDir(QStringLiteral("/proc/%1/fd").arg(pid));
        const auto fdEntries = fdDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::AllDirs | QDir::Files);

        for (const QString &fd : fdEntries) {
            const QByteArray fdPath = fdDir.filePath(fd).toLocal8Bit();

            char targetBuffer[PATH_MAX];
            const ssize_t targetLen = readlink(fdPath.constData(), targetBuffer, sizeof(targetBuffer) - 1);
            if (targetLen < 0) {
                // fd vanished (race) or we lack permission to inspect this process - skip it
                continue;
            }
            targetBuffer[targetLen] = '\0';

            const QString target = QString::fromLocal8Bit(targetBuffer, static_cast<int>(targetLen));
            if (target.startsWith(QLatin1String("pipe:")) || target.startsWith(QLatin1String("socket:"))
                || target.startsWith(QLatin1String("anon_inode:"))) {
                continue;
            }

            if (target.startsWith(path) && !QFileInfo(target).isDir()) {
                openFiles.insert(target);
            }
        }
    }

    return openFiles;
#elif defined(Q_OS_MACOS)
    QSet<QString> openFiles;
    if (path.isEmpty()) {
        return openFiles;
    }

    const pid_t currentPid = getpid();

    int pidBufferSize = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    if (pidBufferSize <= 0) {
        return openFiles;
    }
    std::vector<pid_t> pids(pidBufferSize / sizeof(pid_t) + 16);
    pidBufferSize = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), static_cast<int>(pids.size() * sizeof(pid_t)));
    if (pidBufferSize <= 0) {
        return openFiles;
    }
    const int pidCount = pidBufferSize / static_cast<int>(sizeof(pid_t));

    for (int i = 0; i < pidCount; ++i) {
        const pid_t pid = pids[i];
        if (pid <= 0 || pid == currentPid) {
            continue;
        }

        int fdBufferSize = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, nullptr, 0);
        if (fdBufferSize <= 0) {
            continue;
        }
        std::vector<struct proc_fdinfo> fdInfos(fdBufferSize / sizeof(struct proc_fdinfo) + 1);
        fdBufferSize = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, fdInfos.data(),
            static_cast<int>(fdInfos.size() * sizeof(struct proc_fdinfo)));
        if (fdBufferSize <= 0) {
            continue;
        }
        const int fdCount = fdBufferSize / static_cast<int>(sizeof(struct proc_fdinfo));

        for (int j = 0; j < fdCount; ++j) {
            if (fdInfos[j].proc_fdtype != PROX_FDTYPE_VNODE) {
                continue;
            }

            struct vnode_fdinfowithpath vnodeInfo;
            const int res = proc_pidfdinfo(pid, fdInfos[j].proc_fd, PROC_PIDFDVNODEPATHINFO, &vnodeInfo, sizeof(vnodeInfo));
            if (res != sizeof(vnodeInfo)) {
                continue;
            }

            const QString target = QString::fromUtf8(vnodeInfo.pvip.vip_path);
            if (!target.isEmpty() && target.startsWith(path) && !QFileInfo(target).isDir()) {
                openFiles.insert(target);
            }
        }
    }

    return openFiles;
#else
    (void)path;
    return {};
#endif
}
