package com.example.myapplication

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.graphics.*
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.IBinder
import kotlinx.android.synthetic.main.activity_omok.*
import kotlin.math.max
import kotlin.math.min
import android.os.Handler
import android.widget.Toast


const val USER_ID : Int = 1

const val LINE_INTERVAL : Float = 25.0f
const val LINE_NUM : Int = 19
const val BOARD_SIZE : Int = (LINE_NUM + 1) * LINE_INTERVAL.toInt() + 1
const val LINE_LENGTH : Float = (LINE_NUM - 1) * LINE_INTERVAL
//val MY_STONE = 1
//val OPPONENT_STONE = 2

var stoneInfo = Array(LINE_NUM, {IntArray(LINE_NUM)})
var turn : Int = 1
var gameOver : Boolean = false
var winner : Int = 1
var countdown : Int = 0
var isServiceOn : Boolean = false
lateinit var connection : ServiceConnection
lateinit var context : Context

class OmokActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_omok)
        myID_textView.text = intent.getStringExtra("id1")
        OpID_textView.text = intent.getStringExtra("id2")

        GlobalVariable.fd = openSwitchFromJNI()
        GlobalVariable.timer_fd = openTimerFromJNI()
        if(GlobalVariable.fd < 0) println("Push_switch OPEN ERROR")
        if(GlobalVariable.timer_fd < 0) println("Omokwatch OPEN ERROR")
        println("Open Finished")
        gameOver = false
        context = this
        turn = 1

        connection = object : ServiceConnection {
            override fun onServiceConnected(name: ComponentName, service: IBinder) {
                val mb = service as MyReaderService.MyBinder
                ms = mb.getService()
                isServiceOn = true //
            }
            override fun onServiceDisconnected(name: ComponentName) {
                isServiceOn = false
            }
        }
        val readerIntent : Intent = Intent(this, MyReaderService::class.java)
        if(!isServiceOn) bindService(readerIntent, connection, Context.BIND_AUTO_CREATE)
        val reader = Thread(ReadDataThread())
        reader.start()
        writeTimerFromJNI(GlobalVariable.timer_fd)

        highlightUser()
        drawBoard()

        /* Set Buttons' onClickListeners */
        ok_btn.setOnClickListener {
            LockOn.move(5)
            highlightUser()
            drawBoard()
            writeTimerFromJNI(GlobalVariable.timer_fd)
            if(gameOver) closeGame()
        }
        left_btn.setOnClickListener {
            LockOn.move(4)
            drawBoard()
        }
        up_btn.setOnClickListener {
            LockOn.move(2)
            drawBoard()
        }
        right_btn.setOnClickListener {
            LockOn.move(6)
            drawBoard()
        }
        down_btn.setOnClickListener {
            LockOn.move(8)
            drawBoard()
        }
    } // End of onCreate()
    override fun onStart() {
        super.onStart()
    }

    fun closeGame(){
        var str = "GAME END!"
        str += if(winner == 1) "BLACK WIN"
        else "WHITE WIN"
        Toast.makeText(context, str, Toast.LENGTH_SHORT).show()
        closeFromJNI(GlobalVariable.timer_fd)
        closeFromJNI(GlobalVariable.fd)
        unbindService(connection)
        isServiceOn = false

        for(i in 0 until LINE_NUM)
            for(j in 0 until LINE_NUM)
                stoneInfo[i][j] = 0

        val intent = Intent(this, MainActivity::class.java)
        startActivity(intent)
    }
    fun drawBoard(){
        val bitmap : Bitmap = Bitmap.createBitmap(BOARD_SIZE, BOARD_SIZE, Bitmap.Config.ARGB_8888)
        val canv = Canvas(bitmap)
        val paint = PaintBoard()

        val backgroundRect = Rect(LINE_INTERVAL.toInt(), LINE_INTERVAL.toInt(),
            BOARD_SIZE - LINE_INTERVAL.toInt(), BOARD_SIZE - LINE_INTERVAL.toInt())
        //#FAAC38
        paint.color = Color.parseColor("#faac38")
        paint.style = Paint.Style.FILL_AND_STROKE
        canv.drawRect(backgroundRect, paint)
        paint.color = Color.BLACK
        paint.style = Paint.Style.STROKE


        for(i in 1..LINE_NUM) {
            val x: Float = i * LINE_INTERVAL
            if( i == 10) paint.strokeWidth = 2f
            canv.drawLine(x, LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, paint)
            canv.drawLine(LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, x, paint)
            if( i == 10) paint.strokeWidth = 1f
        }
        for(i in 0 until LINE_NUM){
            for(j in 0 until LINE_NUM){
                if(stoneInfo[i][j] == 1){
                    val rect = getOval(i, j)
                    paint.style = Paint.Style.FILL_AND_STROKE
                    canv.drawOval(rect, paint)
                    paint.style = Paint.Style.STROKE
                } else if(stoneInfo[i][j] == 2){
                    val rect = getOval(i, j)
                    paint.style = Paint.Style.FILL_AND_STROKE
                    paint.color = Color.WHITE
                    canv.drawOval(rect, paint)
                    paint.style = Paint.Style.STROKE
                    paint.color = Color.BLACK
                }
            }
        }
        LockOn.calcRect()
        canv.drawRect(LockOn.rect, LockOn.Painter)
        imageView.setImageBitmap(bitmap)
    }
    private fun getOval(line_x : Int, line_y : Int) : RectF { // line_x : 0~18
        val x = LINE_INTERVAL + line_x * LINE_INTERVAL
        val y = LINE_INTERVAL + line_y * LINE_INTERVAL

        val left = (x - LINE_INTERVAL/2)
        val top = (y - LINE_INTERVAL/2)
        val right = (x + LINE_INTERVAL/2)
        val bottom = (y + LINE_INTERVAL/2)
        return RectF(left, top, right, bottom)
    }
    class PaintBoard : Paint(){
        init{
            this.color = Color.BLACK
            this.strokeWidth = 1.0f
            this.style = Style.STROKE
        }
    }

    object LockOn {
        var rowIdx : Int = LINE_NUM / 2 // rowIdx : 0 ~ 18
        var colIdx : Int = LINE_NUM / 2
        var x_coord : Float = 0f
        var y_coord : Float = 0f
        lateinit var  rect : Rect

        fun initPosition(){ // Center idx : (9, 9)
            rowIdx = LINE_NUM / 2
            colIdx = LINE_NUM / 2
        }
        private fun calcCoord(){
            x_coord = LINE_INTERVAL + rowIdx * LINE_INTERVAL
            y_coord = LINE_INTERVAL + colIdx * LINE_INTERVAL
        }
        fun calcRect(){
            calcCoord()
            val left : Int = (x_coord - LINE_INTERVAL/2).toInt()
            val top : Int = (y_coord - LINE_INTERVAL/2).toInt()
            val right : Int = (x_coord + LINE_INTERVAL/2).toInt()
            val bottom : Int = (y_coord + LINE_INTERVAL/2).toInt()
            rect = Rect(left, top, right, bottom)
        }
        object Painter : Paint(){
            init{
                this.color = Color.RED
                this.strokeWidth = 3.0F
                this.style = Style.STROKE
            }
        }
        fun move(switch_num : Int){
            when(switch_num) {
                2 -> { // UP
                    colIdx = max(colIdx - 1, 0)
                }
                5 -> { // Placement
                    if(stoneInfo[rowIdx][colIdx] == 0) {
                        stoneInfo[rowIdx][colIdx] = turn
                        gameOver = isGameOver(rowIdx, colIdx)
                        if(gameOver) {
                            winner = turn
                            println("G A M E O V E R")
                        }
                        turn = 3 - turn
                        initPosition()
                    }
                }
                4 -> { // LEFT
                    rowIdx = max(rowIdx - 1, 0)
                }
                6 -> { // RIGHT
                    rowIdx = min(rowIdx + 1, LINE_NUM - 1)
                }
                8 -> { // DOWN
                    colIdx = min(colIdx + 1, LINE_NUM - 1)
                }
            }
        }
        fun isGameOver(row : Int, col : Int) : Boolean {
            var cnt = 1
            var cr : Int = row
            var cc : Int = col
            while(++cr < LINE_NUM && stoneInfo[cr][col] == turn) cnt++
            cr = row
            while(--cr >=0 && stoneInfo[cr][col] == turn) cnt++
            if(cnt >= 5) return true

            cnt = 1
            while(++cc < LINE_NUM && stoneInfo[row][cc] == turn) cnt++
            cc =  col
            while(--cc >= 0 && stoneInfo[row][cc] == turn) cnt++
            if(cnt >= 5) return true

            cnt = 1
            cr = row; cc = col
            while(--cr >= 0 && --cc >= 0 && stoneInfo[cr][cc] == turn) cnt++
            cr = row; cc = col
            while(++cr < LINE_NUM && ++cc < LINE_NUM && stoneInfo[cr][cc] == turn) cnt++
            if(cnt >= 5) return true

            cnt = 1
            cr = row; cc = col
            while(--cr >= 0 && ++cc < LINE_NUM && stoneInfo[cr][cc] == turn) cnt++
            cr = row; cc = col
            while(++cr < LINE_NUM && --cc >= 0 && stoneInfo[cr][cc] == turn) cnt++
            if(cnt >= 5) return true

            return false
        }
        fun moved() : Boolean{
            return stoneInfo[rowIdx][colIdx] == 0
        }
    }

    private fun highlightUser(){
        /* If it's my turn, paint myID_textView Yellow.
         * else paint OpID_textView Yellow. */
        if(turn == 1) {
            myID_textView.setBackgroundColor(Color.YELLOW)
            OpID_textView.setBackgroundColor(Color.WHITE)
        } else {
            myID_textView.setBackgroundColor(Color.WHITE)
            OpID_textView.setBackgroundColor(Color.YELLOW)
        }
    }
    private inner class ReadDataThread : Runnable{
        var handler = Handler()

        override fun run() {
            var pressed : Boolean = false
            Thread.sleep(1000)
            while(!gameOver){
                println("reader : " + pressed)
                Thread.sleep(100)
                pressed = ms.getSwitchPressed()
                countdown = readTimerFromJNI(GlobalVariable.timer_fd) - 1
                if(countdown == -1){
                    turn = 3 - turn
                    writeTimerFromJNI(GlobalVariable.timer_fd)
                    handler.post {
                        highlightUser()
                    }
                }
                if(pressed) {
                    var data = ms.popData()
                    println("Read Data : $data")
                    ms.setSwitchPressed()
                    if(data == 5 && LockOn.moved()) writeTimerFromJNI(GlobalVariable.timer_fd)
                    LockOn.move(data)

                    pressed = false
                    handler.post {
                        drawBoard()
                        highlightUser()
                        if(gameOver) closeGame()
                    }
                }
            }
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun openSwitchFromJNI() : Int
    external fun openTimerFromJNI() : Int
    external fun readTimerFromJNI(fd : Int) : Int
    external fun writeTimerFromJNI(fd : Int) : Int
    external fun closeFromJNI(fd : Int) : Void
}

