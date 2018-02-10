
1、The lib directory includes libraries for android platform, armv7 is the only cpu arch supported now. 
   copy the library to /system/lib/ before run any sample.

2、The sample directory includes a sample,
   The sub directory binary_for_armeabi-v7a includes executable files which contains a console application and an apk for armeabi-v7a,
   The sub directory binary_for_armeabi includes executable files which contains a console application and an apk for armeabi,
   The sub directory binary_for_arm64-v8a includes executable files which contains a console application and an apk for arm64-v8a,
   The sub directory Simple_FetchFrame/ includes source code of console application,
   The sub directory Simple_FetchFrame_Apk/ includes source code of an apk.


Note: 1、Change permission of usb device first if application doesn't run as root
      2、The sample apk is developed base on OpenCV, download OpenCV SDK for android is necessary before compiling apk.
      3、If you want to test the apk in binary_for_xxxx directory, please install OpenCV_3.0.0_Manager_3.00_xxxx.apk first
      4、API document refer to https://github.com/percipioxyz/camport2/tree/master/Doc
