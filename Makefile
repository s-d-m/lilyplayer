all clean scan-build:
	${MAKE} -C ./src "$@"

appimage: all
	./make-appimage.sh

.PHONY: all clean scan-build appimage
