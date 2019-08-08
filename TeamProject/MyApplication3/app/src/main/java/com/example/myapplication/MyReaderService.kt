package com.example.myapplication

import android.app.Service
import android.content.Intent
import android.os.Binder
import android.os.IBinder

class MyReaderService : Service() {

    internal var switchPressed : Boolean = false
    internal var data = 0
    private lateinit var reader : Thread
    private var mBinder : IBinder = MyBinder()
    private var isEnd = false
    inner class MyBinder : Binder() {
        fun getService() : MyReaderService{
            return this@MyReaderService
        }
//        val service : MyReaderService
//            get() = this@MyReaderService
    }
    fun popData() : Int {
        val tmp = data
        data = 0
        return tmp
    }
    fun setSwitchPressed() {
        switchPressed = false
    }
    fun getSwitchPressed() : Boolean {
        return switchPressed
    }
    override fun onCreate() {
        super.onCreate()
        reader = Thread(Reader())
        reader.start()
    }
    override fun onBind(intent: Intent): IBinder {
        println("Bind!!!")
        switchPressed = false
        data = 0
        isEnd = false
        return mBinder
    }

    override fun onUnbind(intent: Intent?): Boolean {
        isEnd = true
        switchPressed = false
        return super.onUnbind(intent)
    }

    private inner class Reader : Runnable{
        override fun run() {
            while(!isEnd){
                Thread.sleep(100)
                println("Pressed : " + switchPressed)
                if(!switchPressed) {
                    var ret = readSwitchFromJNI2(GlobalVariable.fd)
                    println("ret : " + ret)
                    if(ret != -1)
                        data = ret
                    else if(data > 0 && ret == -1)
                        switchPressed = true
                }
            }
        }
    }
    external fun readSwitchFromJNI2(fd : Int) : Int
}