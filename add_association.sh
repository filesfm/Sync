#/bin/bash

FILE1=~/.local/share/applications/filesfm.desktop
FILE2=~/.local/share/mime/packages/x-filesfm.xml
for f in $FILE1 $FILE2; do
    [[ -f "$f" ]] && exit 0
done

PROJECT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
echo "[Desktop Entry]
Name=Files.fm
Exec=${PROJECT_DIR}/build/bin/files_fm_sync %f
Type=Application
Terminal=false
MimeType=application/x-filesfm;" >~/.local/share/applications/filesfm.desktop

echo '<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="application/x-filesfm">
    <comment>Files.fm file</comment>
    <glob pattern="*.filesfm"/>
  </mime-type>
</mime-info>' >~/.local/share/mime/packages/x-filesfm.xml

update-mime-database ~/.local/share/mime
xdg-mime default filesfm.desktop application/x-filesfm
