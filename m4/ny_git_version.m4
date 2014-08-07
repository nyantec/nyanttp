AC_DEFUN([NY_GIT_VERSION], [
	AC_REQUIRE([AC_PROG_SED])
	AC_MSG_CHECKING([Build identifier])
	if [[ -d .git ]]
	then
		NY_VERSION_BUILD=$(git describe --dirty --long --always)
	else
		NY_VERSION_BUILD=m4_esyscmd_s(git describe --dirty --long --always)
	fi
	AC_MSG_RESULT([$NY_VERSION_BUILD])

	AC_MSG_CHECKING([Major version])
	NY_VERSION_MAJOR=$(echo $NY_VERSION_BUILD | sed ['s/^v\([0-9]*\).*/\1/'])
	AC_MSG_RESULT([$NY_VERSION_MAJOR])

	AC_MSG_CHECKING([Minor version])
	NY_VERSION_MINOR=$(echo $NY_VERSION_BUILD | sed ['s/^v[0-9]\.\([0-9]*\).*/\1/'])
	AC_MSG_RESULT([$NY_VERSION_MINOR])

	AC_MSG_CHECKING([Micro version])
	NY_VERSION_MICRO=$(echo $NY_VERSION_BUILD | sed ['s/^v[0-9]\.[0-9]-\([0-9]*\).*/\1/'])
	AC_MSG_RESULT([$NY_VERSION_MICRO])

	NY_VERSION=$NY_VERSION_MAJOR.$NY_VERSION_MINOR.$NY_VERSION_MICRO
])
