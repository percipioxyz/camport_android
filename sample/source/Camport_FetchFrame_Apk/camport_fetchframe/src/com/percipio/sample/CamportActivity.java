package com.percipio.sample;

import java.util.HashMap;
import java.util.Iterator;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;

public class CamportActivity extends Activity implements CvCameraViewListener2 {
    private static final String    TAG = "PERCIPIO_CAMERA"; 
    private Mat                    mDepth, mColor;
    final Bitmap bitmap = Bitmap.createBitmap(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT, Bitmap.Config.RGB_565);
    final Bitmap bitmap_color = Bitmap.createBitmap(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT, Bitmap.Config.RGB_565);
    private Button 				   mOpenButton;
    private Button 				   mCloseButton;
    private int                    mRun = 0;
    public static final int DEFAULT_PREVIEW_WIDTH = 640;
    public static final int DEFAULT_PREVIEW_HEIGHT = 480;	
    private ImageView mImageView, mImageView_Color;
    private PendingIntent mPermissionIntent;
    private static final String ACTION_USB_PERMISSION = "com.Android.example.USB_PERMISSION";
	private UsbManager mUsbManager;
	private boolean mDeviceOpenPending = false;
	private long DepthAddr;
    
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("mixed_sample");
                    mDepth = new Mat(DEFAULT_PREVIEW_HEIGHT, DEFAULT_PREVIEW_WIDTH, CvType.CV_8UC3);
                    mColor = new Mat(DEFAULT_PREVIEW_HEIGHT, DEFAULT_PREVIEW_WIDTH, CvType.CV_8UC3);
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
    	@Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
  	                	    Log.d(TAG, "Granted");
  	                        try {                   
  	                	    UsbDeviceConnection connection = ((UsbManager) context.getSystemService(Context.USB_SERVICE)).openDevice(device);                	   
  	                        if (connection == null)
  	                        {
  	    	                    Log.d(TAG, "connection is null");
  	                            return;
  	                        }
  	                	    //OpenDevice();
  	                        } catch (Exception ex) {                                                                                                                                                                                          
  	                            Log.d(TAG, "Can't open device though permission was granted: " + ex);                   
                            }
                        }
                    } else {
                        //showTmsg("用户不允许USB访问设备，程序退出！");
                        finish();
                    }
                }
            }
        }
    };
    
    public CamportActivity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.deep_camera);
        mImageView = (ImageView)findViewById(R.id.deep_camera_view);
        mImageView_Color = (ImageView)findViewById(R.id.color_view);
        mOpenButton = (Button)findViewById(R.id.open_button);
    	mOpenButton.setOnClickListener(mOpenListener);	
        mOpenButton = (Button)findViewById(R.id.close_button);
    	mOpenButton.setOnClickListener(mCloseListener);	
    }

	private final OnClickListener mOpenListener = new OnClickListener() {
		@Override
		public void onClick(final View view) {
			if (mRun == 0) {  
				mDeviceOpenPending = false;
		        OpenDevice();
                mRun = 1;
		        getDataThread.start();	
			} else if ( mRun == 2) {
				 OpenDevice();
				 mRun = 1;	
			}
		}
	};
	
	private final OnClickListener mCloseListener = new OnClickListener() {
		@Override
		public void onClick(final View view) {		
	        mRun = 2;
	        CloseDevice();
	        finish();
	        System.exit(0);
		}
	};

    @Override
    public void onPause()
    {
        super.onPause();
    }

    @Override
    public void onResume()
    {
        Log.i(TAG, "called onResume");
        super.onResume();    
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);     
        
        if (mDeviceOpenPending) {
        	return;
        }
        
    	mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
    	IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
    	registerReceiver(usbReceiver, filter);
    	
   	    mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        HashMap<String,UsbDevice> deviceHashMap = mUsbManager.getDeviceList();
        Iterator<UsbDevice> iterator = deviceHashMap.values().iterator();
        while (iterator.hasNext()) {
            UsbDevice device = iterator.next();
            Log.d(TAG, "Device Name = " + device.getDeviceName());
            Log.d(TAG, "Device Vendor Id = " + device.getVendorId());
            Log.d(TAG, "Device Product Id = " + device.getProductId());
            if(device.getProductId() == 4099 && device.getVendorId() == 1204) {
                mDeviceOpenPending = true;
                mUsbManager.requestPermission(device,mPermissionIntent);
            }
        }
    }

    public void onDestroy() {
        super.onDestroy();
    }

    private final Runnable mUpdateImageTask = new Runnable() {
	@Override
	public void run() {
		synchronized (bitmap) {
			Log.d(TAG, "mIFrameCallback set image");
			mImageView.setImageBitmap(bitmap);
		}
	}
    };	
    
    private final Runnable mUpdateImageTask_color = new Runnable() {
	@Override
	public void run() {
		synchronized (bitmap_color) {
			Log.d(TAG, "mIFrameCallback set image");
			mImageView_Color.setImageBitmap(bitmap_color);
		}
	}
    };
    
    GetDataThread getDataThread = new GetDataThread();
	
    protected class GetDataThread extends Thread {
        @Override
        public void run() {
        	while (true) {
        		if (mRun == 1) {
                	Log.d(TAG, "fatch data ......");
                	FetchData(mDepth.getNativeObjAddr(), mColor.getNativeObjAddr());
                 	//Log.d(TAG, "fatch data ......0");
                	Utils.matToBitmap(mDepth, bitmap);
                	Utils.matToBitmap(mColor, bitmap_color);
                 	//Log.d(TAG, "fatch data ......1");
                	mImageView.post(mUpdateImageTask);
                	mImageView_Color.post(mUpdateImageTask_color);
                   	Log.d(TAG, "fatch data ...... end");          			
        		}
        	}
        }
    }	
    
    public native void CloseDevice();
    public native void OpenDevice();
    public native void FetchData(final long matAddr, final long matAddr_color);

	@Override
	public void onCameraViewStarted(int width, int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onCameraViewStopped() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// TODO Auto-generated method stub
		return null;
	}
}
