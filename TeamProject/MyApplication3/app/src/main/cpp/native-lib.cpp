#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_OmokActivity_openSwitchFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    jint ret = open("/dev/fpga_push_switch", O_RDWR);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_MyReaderService_readSwitchFromJNI2(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    unsigned char buf[9];
    jint i;
    read(fd, buf, sizeof(buf));
    for(i=0; i<9; i++) {
        if (buf[i] == 1) {
            return i + 1;
        }
    }
    return -1;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_OmokActivity_openTimerFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    int ret = open("/dev/omokwatch", O_RDWR);
    return ret;
}
extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_OmokActivity_writeTimerFromJNI(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    unsigned char buf[2] = {0,};
    int ret = write(fd, buf, 2);
    return ret;
}
extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_OmokActivity_readTimerFromJNI(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    char buf = 0;
    int ret = read(fd, &buf, 1);
    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_OmokActivity_closeFromJNI(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    close(fd);
}