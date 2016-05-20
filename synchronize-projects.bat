@echo off

set SYNCHRONIZER_JAR=contrib\VSCppProjectSynchronizer\VSCppProjectSynchronizer.jar

echo Synchronizing VS2008 project files with VS2005 ...

rem Synchronize sipXcallLib and sipXtapi projects
java -jar %SYNCHRONIZER_JAR% sipXcallLib\sipXcallLib-msvc9.vcproj sipXcallLib\sipXcallLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXcallLib\sipXcallLibTest-msvc9.vcproj sipXcallLib\sipXcallLibTest-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXcallLib\sipXtapi-msvc9.vcproj sipXcallLib\sipXtapi-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXcallLib\sipXtapiTest-msvc9.vcproj sipXcallLib\sipXtapiTest-msvc8.vcproj

rem Synchronize sipXmediaAdapterLib projects
java -jar %SYNCHRONIZER_JAR% sipXmediaAdapterLib\sipXmediaAdapterLib-msvc9.vcproj sipXmediaAdapterLib\sipXmediaAdapterLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXmediaAdapterLib\sipXmediaAdapterLibTest-msvc9.vcproj sipXmediaAdapterLib\sipXmediaAdapterLibTest-msvc8.vcproj

rem Synchronize sipXmediaLib projects
java -jar %SYNCHRONIZER_JAR% sipXmediaLib\sipXmediaLib-msvc9.vcproj sipXmediaLib\sipXmediaLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXmediaLib\sipXmediaLibTest-msvc9.vcproj sipXmediaLib\sipXmediaLibTest-msvc8.vcproj

rem Synchronize sipXportLib projects
java -jar %SYNCHRONIZER_JAR% sipXportLib\sipXportLib-msvc9.vcproj sipXportLib\sipXportLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXportLib\sipXportLibTest-msvc9.vcproj sipXportLib\sipXportLibTest-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXportLib\sipXunitTestSuite-msvc9.vcproj sipXportLib\sipXunitTestSuite-msvc8.vcproj

rem Synchronize sipXsdpLib projects
java -jar %SYNCHRONIZER_JAR% sipXsdpLib\sipXsdpLib-msvc9.vcproj sipXsdpLib\sipXsdpLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXsdpLib\sipXsdpLibTest-msvc9.vcproj sipXsdpLib\sipXsdpLibTest-msvc8.vcproj

rem Synchronize sipXtackLib projects
java -jar %SYNCHRONIZER_JAR% sipXtackLib\sipXtackLib-msvc9.vcproj sipXtackLib\sipXtackLib-msvc8.vcproj
java -jar %SYNCHRONIZER_JAR% sipXtackLib\sipXtackLibTest-msvc9.vcproj sipXtackLib\sipXtackLibTest-msvc8.vcproj

echo Done.
