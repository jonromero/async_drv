erldir="/usr/local/lib/erlang"

cppflags="-DNDEBUG -DTERMCOL -I${erldir}/usr/include -I${erldir}/lib/erl_interface-3.7.6/include"
cflags="-Wall -Wextra"
ldflags="-L${erldir}/lib/erl_interface-3.7.6/lib"

gcc $cppflags $cflags -o async_drv.so -fPIC -shared async_drv.c -lerl_interface -lei $ldflags
