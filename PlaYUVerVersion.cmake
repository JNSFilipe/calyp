
include(cmake/GetGitRevisionDescription.cmake)

git_describe(__version --tags)

string(REPLACE "-" ";" __version_list ${__version})
list(GET __version_list 0 __version)
list(GET __version_list 1 __patch)
string(REPLACE "." ";" __version_list ${__version})
list(GET __version_list 0 __major)
list(GET __version_list 1 __minor)


SET( PLAYUVER_VERSION_MAJOR ${__major})
SET( PLAYUVER_VERSION_MINOR ${__minor})
SET( PLAYUVER_VERSION_PATCH ${__patch})
SET( PLAYUVER_VERSION "${PLAYUVER_VERSION_MAJOR}.${PLAYUVER_VERSION_MINOR}.${PLAYUVER_VERSION_PATCH}")

unset(__version_list) 
unset(__version)
unset(__patch)
unset(__major)
unset(__minor)

