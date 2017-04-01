
1、The lib directory includes libraries for android platform, armv7 is the only cpu arch supported now. 
   copy the library to /system/lib/ before run any sample.

2、The sample directory includes a sample,
   The sub directory binary_for_ARMV7 includes executable files which contains a console application and an apk for armv7,
   The sub directory Simple_FetchFrame/ includes source code of console application,
   The sub directory Simple_FetchFrame_Apk/ includes source code of an apk.


Note: 1、Change permission of usb device first if application doesn't run as root
      2、The sample apk is developed base on OpenCV, download OpenCV SDK for android is necessary before compiling apk.
      3、If you want to test the apk in binary_for_ARMV7 directory, please install OpenCV_2.4.9_Manager_2.18_armeabi.apk first
