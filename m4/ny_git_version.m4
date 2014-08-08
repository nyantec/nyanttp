AC_DEFUN([NY_GIT_BUILD], m4_esyscmd_s(git describe --dirty --long --always))
AC_DEFUN([NY_GIT_VERSION], m4_esyscmd_s(echo NY_GIT_BUILD | sed ['s/^v\([0-9]*\)\.\([0-9]*\)-\([0-9]*\).*/\1.\2.\3/'])))
AC_DEFUN([NY_GIT_LIBVER], m4_esyscmd_s([expr $(git tag -l 'v*.*' | wc -l) - 1]):m4_esyscmd_s([git rev-list $(git describe --abbrev=0 --tags).. --count]):m4_esyscmd_s([expr $(git tag -l $(git describe --abbrev=0 --tags | sed 's/^\(v[0-9]*\).*/\1.*/') | wc -l) - 1]))
