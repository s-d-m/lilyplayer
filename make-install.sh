#!/bin/bash

this_app_name='lilyplayer'


function create_dir()
{
    local readonly dir_to_create="$1"
    mkdir -p "${dir_to_create}"
    if [ ! -d "${dir_to_create}" ] ; then
	printf >&2 'Error could not create directory %s\n' "${dir_to_create}"
	return 2
    fi
}

function install()
{
    local readonly dest_dir="$1"
    if [ -z "$dest_dir" ] ; then
	printf >&2 '%s\n' 'Error variable dest_dir is unset'
	exit 2
    fi

    if (! create_dir "${dest_dir}") ; then
	return 2;
    fi

    for path in 'usr/bin/' 'usr/lib/' 'usr/share/applications' 'usr/share/applications/hicolor/256x256/apps/' ; do
	if (! create_dir "${dest_dir}/${path}") ; then
	    return 2;
	fi
    done

    cp -- "./bin/${this_app_name}" "${dest_dir}/usr/bin"
    find ./3rd-party/rtmidi/.libs/ -name 'librtmidi.so*' -exec cp '--' '{}' "${dest_dir}/usr/lib" ';'
    cp -- './misc/lilyplayer.desktop' "${dest_dir}/usr/share/applications/"
    cp -- './misc/logo_hicolor_256x256.png' "${dest_dir}/usr/share/applications/hicolor/256x256/apps/lilyplayer.png"
}

install "$@"
