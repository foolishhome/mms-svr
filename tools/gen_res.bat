mc -U yysvr_res.mc
rc -r yysvr_res.rc
link -dll -noentry -out:%SYSTEMROOT%\System32\yysvr_res.dll yysvr_res.res
copy yysvr_res.h ..\include\

regedit /S yysvr_res.reg
