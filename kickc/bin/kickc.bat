@ECHO OFF
REM JAVA HOME
REM KICKC HOME
set KICKC_HOME=%~dp0..
echo KICKC_HOME=%KICKC_HOME% 
REM KCLIB HOME
set KICKCLIB_HOME=%KICKC_HOME%\stdlib
echo KICKCLIB_HOME=%KICKCLIB_HOME%
set KICKC_FRAGMENT_HOME=%KICKC_HOME%\fragment
echo KICKC_FRAGMENT_HOME=%KICKC_FRAGMENT_HOME%
REM KICKASSEMBLER HOME
REM VICE HOME
REM KICKC_JAR
for %%I in ( %KICKC_HOME%\lib\kickc-*.jar ) do set KICKC_JAR=%%I
echo KICKC_JAR=%KICKC_JAR%

echo java -jar %KICKC_JAR% -I %KICKCLIB_HOME% -F %KICKC_FRAGMENT_HOME% %*
java -jar %KICKC_JAR% -I %KICKCLIB_HOME% -F %KICKC_FRAGMENT_HOME% %*