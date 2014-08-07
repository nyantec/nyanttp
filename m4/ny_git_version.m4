AC_DEFUN([NY_GIT_VERSION], m4_esyscmd_s([git describe --dirty --long --always | sed 's/^v\([0-9]*\)\.\([0-9]*\)-\([0-9]*\).*/\1.\2.\3/'])))
