all:
clean:
scan-build:
%:
	${MAKE} -C ./src "$@"
