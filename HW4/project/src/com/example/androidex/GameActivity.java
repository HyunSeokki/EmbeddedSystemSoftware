package com.example.androidex;

import java.util.Random;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class GameActivity extends Activity implements View.OnClickListener{
	
	LinearLayout linear;
	int row_size = 0;
	int col_size = 0;
	int height = 360;
	int width = 1024;
	int blank = 0;
	int[] num = new int[26];
	private IMyTimerService binder;	
	private TextView tv;
	private boolean running = false;
	String str;
	String[] parseStr;
	Random pos_generator = new Random();
	Random num_generator = new Random();
		
	private ServiceConnection connection = new ServiceConnection(){
		@Override
		public void onServiceConnected(ComponentName name, IBinder service){
			binder = IMyTimerService.Stub.asInterface(service);
		}
		
		@Override
		public void onServiceDisconnected(ComponentName name){
			
		}
	};
	
	/* Function Name : clearButtons
	 * Type : public void
	 * Parameter : int num
	 * Contents : remove buttons
	 */
	public void clearButtons(int num){		
		for(int i=0; i < num; i++)
		{
			LinearLayout childView = (LinearLayout)linear.getChildAt(3);
			childView.removeAllViewsInLayout();
			linear.removeView(childView);
		}
	}
		
	/* Function Name : makeButtons
	 * Type : public void
	 * Parameter : void
	 * Contents : make buttons using number that you input
	 */
	public void makeButtons(){
		int random_pos = pos_generator.nextInt((row_size * col_size));
		int random_num;
		int [] temp = new int[26];
		
		for(int i=0;i<row_size;i++)
		{
			LinearLayout row = new LinearLayout(this);
			row.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT));
			random_num = num_generator.nextInt((row_size * col_size) - 1) + 1;
			
			for(int j=0;j<col_size;j++)
			{
				Button btnTag = new Button(this);
				btnTag.setId(i*col_size + j);
				if((i * col_size + j) == random_pos)
				{
					btnTag.setBackgroundColor(Color.BLACK);
					btnTag.setLayoutParams(new LayoutParams(width / col_size, height / row_size));
					blank = i * col_size + j;
				}
				else
				{	
					while(temp[random_num] != 0)
						random_num = num_generator.nextInt((row_size * col_size) - 1) + 1;
					
					btnTag.setLayoutParams(new LayoutParams(width / col_size, height / row_size));
					btnTag.setText(String.valueOf(random_num));
					temp[random_num] = 1;
					num[i*col_size + j] = random_num;
					
					btnTag.setOnClickListener(this);
				}							
				row.addView(btnTag);
			}
			linear.addView(row);					
		}
	}	
	
	/* Function Name : moveButtons
	 * Type : public void
	 * Parameter : void
	 * Contents : clear buttons and remake buttons
	 */
	public void moveButtons(){
		for(int i=0;i<row_size;i++)
		{
			LinearLayout row = new LinearLayout(this);
			row.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT));
			
			for(int j=0;j<col_size;j++)
			{
				Button btnTag = new Button(this);
				btnTag.setId(i*col_size + j);
				if(i * col_size + j == blank)
				{
					btnTag.setBackgroundColor(Color.BLACK);
					btnTag.setLayoutParams(new LayoutParams(width / col_size, height / row_size));
				}
				else
				{											
					btnTag.setLayoutParams(new LayoutParams(width / col_size, height / row_size));
					btnTag.setText(String.valueOf(num[i*col_size+j]));
					btnTag.setOnClickListener(this);
				}					
				row.addView(btnTag);
			}
			linear.addView(row);					
		}
	}
	
	/* Function Name : onClick
	 * Type : public void
	 * Parameter : View v
	 * Contents : button event that push puzzle
	 */
	public void onClick(View v){
		Button data = (Button)v;
		int blank_row, blank_col, btn_row ,btn_col;
		int temp_num;
		blank_row = blank / col_size;
		blank_col = blank % col_size;
		btn_row = data.getId() / col_size;
		btn_col = data.getId() % col_size;
		
		if(blank_row == btn_row && blank_col-1 == btn_col) // left
		{
			clearButtons(row_size);
			temp_num = num[blank];
			num[blank] = num[data.getId()];
			num[data.getId()] = temp_num;
			blank-=1;
			moveButtons();
		}
		else if(blank_row == btn_row && blank_col+1 == btn_col) // right
		{
			clearButtons(row_size);
			temp_num = num[blank];
			num[blank] = num[data.getId()];
			num[data.getId()] = temp_num;
			blank+=1;
			moveButtons();
		}
		else if(blank_col == btn_col && blank_row-1 == btn_row) // up
		{
			
			clearButtons(row_size);
			temp_num = num[blank];
			num[blank] = num[data.getId()];
			num[data.getId()] = temp_num;
			blank-=col_size;
			moveButtons();
		}
		else if(blank_col == btn_col && blank_row+1 == btn_row) // down
		{
			clearButtons(row_size);
			temp_num = num[blank];
			num[blank] = num[data.getId()];
			num[data.getId()] = temp_num;
			blank+=col_size;
			moveButtons();
		}
		
		check();
	}
	
	/* Function Name : IsNumber
	 * Type : public boolean
	 * Parameter : String str_num
	 * Contents : check whether parameter is number or not 
	 */
	public boolean IsNumber(String str_num){
		try{
			int str = Integer.parseInt(str_num);
		}
		catch(NumberFormatException e){
			return false;
		}
		return true;
	}
	
	/* Function Name : check
	 * Type : public void
	 * Parameter : void
	 * Contents : check puzzle whether order is correct or not
	 */
	public void check(){
		for(int i=0;i<col_size;i++)
		{
			for(int j=0;j<row_size;j++)
			{				
				if(blank == i*row_size+j && blank != (row_size * col_size - 1))
					return;
				else if(blank == i*row_size+j && blank == (row_size * col_size - 1))
					break;
				else if(num[i*row_size+j] != i*row_size+j+1)
					return;				
			}
		}
		Intent intent = new Intent(GameActivity.this, MyTimerService.class);
		unbindService(connection); // service end
		running = false;
		
		finish();
	}
	
	/* Function Name : onCreate
	 * Type : public void
	 * Parameter : Bundle savedInstanceState
	 * Contents : the event that runs when the activity is first called.
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_game);		
		linear = (LinearLayout)findViewById(R.id.container);
		linear.setOrientation(LinearLayout.VERTICAL);
		Button btn=(Button)findViewById(R.id.button1);
		tv = (TextView)findViewById(R.id.textView1);
		
		OnClickListener listener=new OnClickListener(){
			public void onClick(View v){
				InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
				EditText data = (EditText)findViewById(R.id.editText1);				
				imm.hideSoftInputFromWindow(data.getWindowToken(), 0);
									
				str = data.getText().toString();
				parseStr = str.split(" ");
				
				if(parseStr.length != 2)
					return;
				if(IsNumber(parseStr[0]) == false || IsNumber(parseStr[1]) == false)
					return;
				
				int temp_row;
				int temp_col;

				temp_row = Integer.parseInt(parseStr[0]);
				temp_col = Integer.parseInt(parseStr[1]);

				if(temp_row > 5 || temp_col > 5 || (temp_row < 2 && temp_col < 2))
					return;
				
				int ex_row = row_size;
				row_size = temp_row;
				col_size = temp_col;
				
				if(ex_row != 0)
				{
					clearButtons(ex_row);					
					Intent intent = new Intent(GameActivity.this, MyTimerService.class);
					unbindService(connection);
					running = false;
				}			
			
				Intent intent = new Intent(GameActivity.this, MyTimerService.class);
				bindService(intent, connection, BIND_AUTO_CREATE); // service start
				running = true;
				new Thread(new GetTimeThread()).start();
				
				makeButtons();
			}
		};
		btn.setOnClickListener(listener);
	}
	
	private class GetTimeThread implements Runnable{
		private Handler handler = new Handler();
		
		@Override
		public void run(){
			while(running){
				if(binder == null)
					continue;
				
				handler.post(new Runnable(){
					@Override
					public void run(){
						try{
							String min = String.format("%02d",binder.getTime() / 60);
							String sec = String.format("%02d",binder.getTime() % 60);
							tv.setText(min + ":" + sec);
							
						}catch(RemoteException e){
							e.printStackTrace();
						}
					}
				});
				
				try{
					Thread.sleep(500);					
				}catch(InterruptedException e){
					e.printStackTrace();
				}
			}
		}
	}
}
