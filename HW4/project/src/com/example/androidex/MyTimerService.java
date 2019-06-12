package com.example.androidex;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;

public class MyTimerService extends Service{
	private boolean isStop;
	private int time;
	
	public MyTimerService(){		
	}

	IMyTimerService.Stub binder = new IMyTimerService.Stub(){
		@Override
		public int getTime() throws RemoteException{
			return time;
		}
	};
	
	@Override
	public void onCreate(){
		super.onCreate();
		
		Thread timer = new Thread(new Timer());
		timer.start();
	}
	
	@Override
	public void onDestroy(){
		super.onDestroy();
		isStop = true;
	}
	
	@Override
	public IBinder onBind(Intent intent) {
		return binder;
	}
	
	@Override
	public boolean onUnbind(Intent intent){
		isStop = true;
		return super.onUnbind(intent);
	}
	
	private class Timer implements Runnable{
		private Handler handler = new Handler();
		
		@Override
		public void run(){
			for(time=0; time < 3600; time++)
			{
				if(isStop)
					break;
				
				handler.post(new Runnable(){
					@Override
					public void run(){
						
					}
				});
				try{
					Thread.sleep(1000);
				} catch(InterruptedException e){
					e.printStackTrace();
				}
			}
			
			handler.post(new Runnable(){
				@Override
				public void run(){
					
				}
			});
		}
	}
}