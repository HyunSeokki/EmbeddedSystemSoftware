package com.example.androidex;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;

public class GameActivity extends Activity{
	
	LinearLayout linear;
	int row_size = 0;
	int col_size = 0;
	String str;
	String[] parseStr;
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
				EditText data = (EditText)findViewById(R.id.editText1);
				
				str = data.getText().toString();
				parseStr = str.split(" ");
				if(parseStr.length != 2)
					return;
				//else if(isStringInt(parseStr[0]) == false)
				row_size = Integer.parseInt(parseStr[0]);
				col_size = Integer.parseInt(parseStr[1]);
			}
		};
		btn.setOnClickListener(listener);
		
		for(int i=0;i<3;i++)
		{
			LinearLayout row = new LinearLayout(this);
			row.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT));
			
			for(int j=0;j<4;j++)
			{
				Button btnTag = new Button(this);
				btnTag.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT));
				btnTag.setText("Button"+(j+1+(i*4)));
				row.addView(btnTag);
			}
			linear.addView(row);
		}
	}

}
