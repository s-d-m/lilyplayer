#!/bin/bash

this_app_name='lilyplayer'

this_dir="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

# too bad linuxdeployqt provides only an x86_64 AppImage, otherwise this script could also make an AppImage
# for other architectures


tmp_dir="$(mktemp -t -d "DIR_TO_MAKE_AN_APPIMAGE_FOR_${THIS_APP_NAME}.XXXXXX")"
function finish()
{
   rm -rf -- "${tmp_dir}"
}

trap finish EXIT

# avoid downloading linuxdeployqt if it is already in the path
function get_linuxdeployqt() {
    local readonly app_name='linuxdeployqt-continuous-x86_64.AppImage'
    local readonly linuxdeployqt="$(which "${app_name}")"
    if [ -x "$linuxdeployqt" ] ; then
	echo "$linuxdeployqt"
    else
	local readonly url_linuxqtdeploy='https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage'

	printf >&2 '\n\n\n[INFO] Could not find linuxdeployqt-continuous-x86_64.AppImage on your system.\n'
	printf >&2 '[INFO] downloading it for you\n'
	printf >&2 '[INFO] pro tipp: to avoid downloading again and again, just download it once from %s, make it executable, and save it a folder that appears in %s\n' "${url_linuxqtdeploy}" "${PATH}"
	printf >&2 '\n\n\n\n\n\n'

	wget --progress=bar:force -O "${tmp_dir}/${app_name}" "${url_linuxqtdeploy}"
	if [ ! -e  "${tmp_dir}/${app_name}" ] ; then
	    echo >&2 'Failed to download "${app_name}" from "${url_linuxqtdeploy}"'
	    exit 2
	fi

	chmod +x  "${tmp_dir}/${app_name}"
	realpath  "${tmp_dir}/${app_name}"
    fi
}


function make_appimage()
{
    pushd "${tmp_dir}"

    if [ ! -e "${this_dir}/bin/${this_app_name}" ] ; then
	echo >&2 'could not find '"${this_dir}/bin/${this_app_name}"'. Did you run make?'
	exit 2
    else
	cp -- "${this_dir}/bin/${this_app_name}" "${tmp_dir}/"
    fi

cat > "${tmp_dir}/default.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=${this_app_name}
Exec=AppRun %F
Icon=default
Comment=plays music sheets
Terminal=false
EOF

    local readonly linuxqtdeploy="$(get_linuxdeployqt)"
    ARCH=x86_64 "${linuxqtdeploy}" "./${this_app_name}" -appimage
    if [ ! -e "./${this_app_name}-x86_64.AppImage" ] ; then
	printf >&2 'Failed to create %s\n' "${this_app_name}-x86_64.AppImage"
	exit 2
    fi

    cp -- "./${this_app_name}-x86_64.AppImage" "${this_dir}/bin/"
    printf 'appimage is available in %s\n'"${this_dir}/bin/${this_app_name}-x86_64.AppImage"
    popd
}

make_appimage
