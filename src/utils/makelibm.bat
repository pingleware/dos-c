tlib c:\borlandc\lib\cs.lib *H_LDIV *H_LLSH *H_LURSH *N_LXMUL *H_LRSH *H_SPUSH *N_SCOPY
tlib libm +H_LDIV +H_LLSH +H_LURSH +N_LXMUL +H_LRSH +H_SPUSH +N_SCOPY
del H_LDIV.OBJ
del H_LLSH.OBJ
del H_LURSH.OBJ
del N_LXMUL.OBJ
del H_LRSH.OBJ
del H_SPUSH.OBJ
del N_SCOPY.OBJ
copy libm.lib ..\..\lib
