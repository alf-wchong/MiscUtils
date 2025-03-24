@echo off
set SERVICE_NAME=<nameOfServiceInRegistry>
set TOMCAT_HOME=<tomcatHomeDirectory>[C:\Tomcat]
set INSTANCE_PATH=<directoryOfThisAdditionalTomcatInstance>[%TOMCAT_HOME%\instanceN]
set JVM_DLL="%JAVA_HOME%\bin\server\jvm.dll"

echo Installing %SERVICE_NAME% service...

"%TOMCAT_HOME%\bin\tomcat10.exe" //IS//%SERVICE_NAME% ^
  --DisplayName="<nameOfServiceToDisplayOnWindowsServiceApplet>" ^
  --Description="<descriptionOfService>" ^
  --Install="%TOMCAT_HOME%\bin\tomcat10.exe" ^
  --Jvm=%JVM_DLL% ^
  --StartMode=jvm ^
  --StopMode=jvm ^
  --StartClass=org.apache.catalina.startup.Bootstrap ^
  --StartParams=start ^
  --StopClass=org.apache.catalina.startup.Bootstrap ^
  --StopParams=stop ^
  --Classpath="%TOMCAT_HOME%\bin\bootstrap.jar;%TOMCAT_HOME%\bin\tomcat-juli.jar" ^
  --JvmOptions=-Dcatalina.home=%TOMCAT_HOME% ^
  --JvmOptions=-Dcatalina.base=%INSTANCE_PATH% ^
  --JvmOptions=-Djava.io.tmpdir=%INSTANCE_PATH%\temp ^
  --JvmOptions=-Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager ^
  --JvmOptions=-Djava.util.logging.config.file=%INSTANCE_PATH%\conf\logging.properties ^
  --JvmOptions9=--add-opens=java.base/java.lang=ALL-UNNAMED ^
  --JvmOptions9=--add-opens=java.base/java.io=ALL-UNNAMED ^
  --JvmOptions9=--add-opens=java.base/java.util=ALL-UNNAMED ^
  --JvmOptions9=--add-opens=java.base/java.util.concurrent=ALL-UNNAMED ^
  --JvmOptions9=--add-opens=java.rmi/sun.rmi.transport=ALL-UNNAMED ^
  --JvmMs=128 ^
  --JvmMx=512 ^
  --LogPath="%INSTANCE_PATH%\logs" ^
  --StdOutput=auto ^
  --StdError=auto

if %ERRORLEVEL% == 0 (
  echo Service %SERVICE_NAME% installed successfully.
  echo You can now start it with: net start %SERVICE_NAME%
) else (
  echo Failed to install service %SERVICE_NAME%.
)
