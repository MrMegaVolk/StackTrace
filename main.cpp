#include <QCoreApplication>

#include <QThread>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>

#include <string>
#include <chrono>
#include "aTrace.h"

QMutex mutex;
static char** functArr;
void process(QString str) {
    aTrace("void process(QString str)");
    while (true) {
        if (false)
        {
            mutex.lock();
            std::cout << std::this_thread::get_id() << std::endl;
            //std::cout << std::thread::get_id() << std::endl;
            qDebug() << QThread::currentThread() << str;
            qDebug() << QThread::currentThreadId() << str;
            mutex.unlock();
        }
        QThread::sleep(3);
    }
    aeTrace();
}

void empt(QString str) {
    aTrace("void empt(QString str)");
    process(str);
    aeTrace();
}
void tempt(QString str) {
    aTrace("void tempt(QString str)");
    empt(str);
    aeTrace();
}

void one(QString str) {
    aTrace("void one(QString str)");
    nmTrace("first thread");
    process(str);
    aeTrace();
}

void two(QString str) {
    aTrace("void two(QString str)");
    nmTrace("second thread");
    empt(str);
    aeTrace();
}

void three(QString str) {
    aTrace("void three(QString str)");
    nmTrace("third thread");
    tempt(str);
    aeTrace();
}

static int gindx = 0;
QMutex mutex2;
int cnt = 1000 - 1; // -1 т.к. главный тред тоже учитывается
int deep = 1000 - 1; // по причине, что функция test уже в стеке
void test() {
    mutex.lock();
    mutex.unlock();
    //std::cout << std::this_thread::get_id() << " ";
    aTrace("void test()");
    nmTrace("test thread");
    for (int j=0;j<deep;j++) { // 1000 дабы не переполнить стек
        //aTrace("some string may be toooooooo loooooooong");
        //mutex2.lock();
        aTrace(functArr[gindx++]); // можно без мьютекса, не столь важно какая строка будет
        //mutex2.unlock();
    }
    for (int j=0;j<deep;j++) { // 1000 дабы не переполнить стек
        aeTrace();
    }
    aeTrace();
}

using namespace std::chrono;
int main()
{
    if (true) {
        functArr = new char*[cnt*deep];
        for (int i = 0; i < cnt*deep; i++) {
            char* ch = new char[10];
            itoa(i, ch, 10);
            functArr[i] = ch;
        }
        unsigned __int64 right = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
        aTrace("int main()");
        nmTrace("mainThread");
        mutex.lock();
        
        QFuture<void>* f = new QFuture<void>[cnt];
        for (int i = 0; i < cnt; i++) // кол-во повторов!
            f[i] = QtConcurrent::run(test);
        mutex.unlock();
        for (int i = 0; i < cnt; i++)
            f->waitForFinished();
        unsigned __int64 left = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
        std::cout << std::endl;
        std::cout << left << " ms" << std::endl;
        std::cout << right << " ms" << std::endl;
        std::cout << (left - right) << " ms" << std::endl;
    }

    //std::cout << std::this_thread::get_id() << std::endl;
    QFuture<void> f1 = QtConcurrent::run(one, QString("One!"));
    QFuture<void> f2 = QtConcurrent::run(two, QString("Two!"));
    QFuture<void> f3 = QtConcurrent::run(three, QString("Three!"));
    
    QThread::sleep(1);
    pTrace();
    aeTrace();
    
    std::cout << "done!" << std::endl;
    std::cout.flush();
    return 0;
}
