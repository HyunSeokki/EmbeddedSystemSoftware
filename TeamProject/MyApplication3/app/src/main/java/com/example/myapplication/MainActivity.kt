package com.example.myapplication

import android.content.Intent
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

object GlobalVariable {
    var fd : Int = 0
    var timer_fd : Int = 0
}
lateinit var ms : MyReaderService

/*
 * /data/local/tmp에 있는 fpgas/device_driver/fpga_insmod.sh 실행할 것.
 */
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


        start_btn.setOnClickListener{
            val intent = Intent(this, OmokActivity::class.java)
            intent.putExtra("id1", id1_editText.text.toString())
            intent.putExtra("id2", id2_editText.text.toString())
            startActivity(intent)
        }

    }
    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
