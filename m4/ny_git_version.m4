AC_DEFUN([NY_GIT_BUILD], m4_esyscmd_s(git describe --dirty --long --always))
AC_DEFUN([NY_GIT_VERSION], m4_esyscmd_s(echo NY_GIT_BUILD | sed ['s/^v\([0-9]*\)\.\([0-9]*\)-\([0-9]*\).*/\1.\2.\3/'])))
