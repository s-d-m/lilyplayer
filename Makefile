all lilyplayer:
	${MAKE} -C ./src "$@"

clean:
	${MAKE} -C ./src "$@"
	${MAKE} -C ./3rd-party/rtmidi/ "$@"

install: lilyplayer
	./make-install.sh ${DESTDIR}

appimage: lilyplayer
	./make-appimage.sh

.PHONY: all lilyplayer clean appimage install
