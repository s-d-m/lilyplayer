#!/bin/bash

set +x

this_app_name='lilyplayer'

this_dir="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

# too bad linuxdeployqt provides only an x86_64 AppImage, otherwise this script could also make an AppImage
# for other architectures


tmp_dir="$(mktemp -t -d "DIR_TO_MAKE_AN_APPIMAGE_FOR_${this_app_name^^}.XXXXXX")"
function finish()
{
  rm -rf -- "${tmp_dir}"
}

trap finish EXIT

# avoid downloading linuxdeployqt if it is already in the path
function get_linuxdeployqt() {
    local readonly app_name='linuxdeployqt-continuous-x86_64.AppImage'
    local readonly linuxdeployqt="$(command -v "${app_name}")"
    if [ -n "$linuxdeployqt" ] && [ -x "$linuxdeployqt" ] ; then
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

function add_workaround_to_increase_compat()
{

    local readonly linuxqtdeploy="$1"

    # Workaround to increase compatibility with older systems; see
    # https://github.com/darealshinji/AppImageKit-checkrt for details

    mkdir -p "${tmp_dir}/usr/optional/"
    wget -c 'https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so' -O "${tmp_dir}/usr/optional/exec.so"
    mkdir -p "${tmp_dir}/usr/optional/libstdc++"
    cp '/usr/lib/x86_64-linux-gnu/libstdc++.so.6' "${tmp_dir}/usr/optional/libstdc++"

    ( rm -f -- "${tmp_dir}/AppRun"
      wget -c 'https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64' -O "${tmp_dir}/AppRun"
      chmod a+x "${tmp_dir}/AppRun"
    )

  # Manually invoke appimagetool so that libstdc++ gets bundled and the modified AppRun stays intact
    pushd "${tmp_dir}"
    "$linuxqtdeploy" --appimage-extract
    cp -- "${tmp_dir}/usr/share/applications/${this_app_name}.desktop"  "${tmp_dir}/"
    cp -- "${tmp_dir}/usr/share/applications/hicolor/256x256/apps/${this_app_name}.png"  "${tmp_dir}/"

    PATH="$(readlink -f "${tmp_dir}/squashfs-root/usr/bin"):${PATH}" "${tmp_dir}/squashfs-root/usr/bin/appimagetool" -g "${tmp_dir}/" "${tmp_dir}/${this_app_name}-x86_64.AppImage"
    popd
}

function make_appimage()
{
    make -C "${this_dir}"
    make -C "${this_dir}" install DESTDIR="${tmp_dir}"


    local readonly linuxqtdeploy="$(get_linuxdeployqt)"
    local readonly version="$(git rev-parse --short HEAD)"

    unset QTDIR
    unset QT_PLUGIN_PATH
    unset LD_LIBRARY_PATH


    ARCH=x86_64 "${linuxqtdeploy}" "${tmp_dir}/usr/share/applications/${this_app_name}.desktop" -appimage -bundle-non-qt-libs
    if [ ! -e "${this_dir}/${this_app_name}-x86_64.AppImage" ] ; then
	printf >&2 'Failed to create %s\n' "${this_app_name}-x86_64.AppImage"
	exit 2
    # else
    # 	mv -- "${this_dir}/${this_app_name}-x86_64.AppImage" "${this_dir}/bin/${this_app_name}-${version}_without_workaround_for_old_systems_x86_64.AppImage"
    fi

    add_workaround_to_increase_compat "$linuxqtdeploy"

    if [ ! -e "${tmp_dir}/${this_app_name}-x86_64.AppImage" ] ; then
	printf >&2 'Failed to add the workaround for older systems\n'
	exit 2
    fi

    local readonly dst_appimage="${this_dir}/bin/${this_app_name}-${version}-x86_64.AppImage"

    rm -f -- "${this_dir}/${this_app_name}-x86_64.AppImage"
    cp -- "${tmp_dir}/${this_app_name}-x86_64.AppImage" "${dst_appimage}"
    cp -- "${tmp_dir}/${this_app_name}-x86_64.AppImage" "${this_dir}/bin/${this_app_name}-x86_64.AppImage"

    printf 'Following libraries are required on the system to run the appimage:\n'
    find "${tmp_dir}" -executable -type f -exec ldd '{}' ';' | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq

    printf 'appimage is available at %s\n' "${dst_appimage}"
}

make_appimage
