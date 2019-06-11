package com.example.androidex;

import java.util.Random;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;

public class GameActivity extends Activity implements View.OnClickListener{
	
	LinearLayout linear;
	int row_size = 0;
	int col_size = 0;
	int height = 390;
	int width = 1024;
	int blank = 0;
	int[] num = new int[26];
	String str;
	String[] parseStr;
	Random pos_generator = new Random();
	Random num_generator = new Random();
	GameActivity gameActivity;
	
	public void clearButtons(int num){
		for(int i=0; i < num; i++)
		{
			LinearLayout childView = (LinearLayout)linear.getChildAt(2);
			childView.removeAllViewsInLayout();
			linear.removeView(childView);
		}
	}
		
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
		
		if(check() == 1)
			finish();
	}
	
	public int check(){
		Log.v("check","start");
		for(int i=0;i<col_size;i++)
		{
			for(int j=0;j<row_size;j++)
			{				
				if(blank == i*row_size+j && blank != (row_size * col_size - 1))
					return 0;
				else if(blank == i*row_size+j && blank == (row_size * col_size - 1))
					break;
				else if(num[i*row_size+j] != i*row_size+j+1)
					return 0;
				
			}
		}

		return 1;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_game);		
		linear = (LinearLayout)findViewById(R.id.container);
		linear.setOrientation(LinearLayout.VERTICAL);
		Button btn=(Button)findViewById(R.id.button1);
				
		OnClickListener listener=new OnClickListener(){
			public void onClick(View v){
				InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
				EditText data = (EditText)findViewById(R.id.editText1);				
				imm.hideSoftInputFromWindow(data.getWindowToken(), 0);
									
				str = data.getText().toString();
				parseStr = str.split(" ");
				
				if(parseStr.length != 2)
					return;
				
				int ex_row = row_size;
				row_size = Integer.parseInt(parseStr[0]);
				col_size = Integer.parseInt(parseStr[1]);

				if(row_size > 5 || col_size > 5 || (row_size == 1 && col_size == 1))
					return;
				
				if(ex_row != 0)
					clearButtons(ex_row);
			
				makeButtons();
			}
		};
		btn.setOnClickListener(listener);
	}
}
