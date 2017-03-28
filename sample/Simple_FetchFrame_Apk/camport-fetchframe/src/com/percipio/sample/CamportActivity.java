package com.percipio.sample;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;

public class CamportActivity extends Activity implements CvCameraViewListener2 {
    private static final String    TAG = "PERCIPIO_CAMERA";
    
    private Mat                    mDepth;
    final Bitmap bitmap = Bitmap.createBitmap(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT, Bitmap.Config.RGB_565);
    private Button 				   mOpenButton;
    private Button 				   mCloseButton;
    private int                    mRun = 0;
    public static final int DEFAULT_PREVIEW_WIDTH = 640;
    public static final int DEFAULT_PREVIEW_HEIGHT = 480;	
    private ImageView mImageView;
    
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("mixed_sample");
                    mDepth = new Mat(DEFAULT_PREVIEW_HEIGHT, DEFAULT_PREVIEW_WIDTH, CvType.CV_8UC1);
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
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
        mOpenButton = (Button)findViewById(R.id.open_button);
    	mOpenButton.setOnClickListener(mOpenListener);	
        mOpenButton = (Button)findViewById(R.id.close_button);
    	mOpenButton.setOnClickListener(mCloseListener);	
    }

	private final OnClickListener mOpenListener = new OnClickListener() {
		@Override
		public void onClick(final View view) {
			if (mRun == 0) {  
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
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
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
    
    GetDataThread getDataThread = new GetDataThread();
	
    protected class GetDataThread extends Thread {
        @Override
        public void run() {
        	while (true) {
        		if (mRun == 1) {
                	Log.d(TAG, "fatch data....");
                	FetchData(mDepth.getNativeObjAddr());
                 	Log.d(TAG, "fatch data....0");
                	Utils.matToBitmap(mDepth, bitmap);
                 	Log.d(TAG, "fatch data....1");
                	mImageView.post(mUpdateImageTask);
                   	Log.d(TAG, "fatch data.... end");          			
        		}
    		
        	}
        }
    }	
    
    public native void CloseDevice();
    public native void OpenDevice();
    public native void FetchData(final long matAddr);

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
