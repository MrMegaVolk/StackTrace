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
        throw 1;
        QThread::sleep(3);
    }
    aeTrace();
}

void on1(QString str) {
    aTrace("void one(QString str)");
    process(str);
    aeTrace();
}
void on2(QString str) {
    aTrace("void one(QString str)");
    on1(str);
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
    on1(str);
    aeTrace();
}
void three(QString str) {
    qDebug() << __FUNCTION__;
    aTrace("void three(QString str)");
    nmTrace("third thread");
    on2(str);
    aeTrace();
}

// Я знаю о существовании __FUNCTION__ но тут я хотел так попробывать
// через доп. массив, дабы строки у одной и той же фун-ции были разные
static int gindx = 0;
QMutex mutex2;
unsigned int cnt = 10 - 1; // -1 т.к. главный тред тоже учитывается
unsigned int deep = 1000 - 2; // по причине, что функция test уже в стеке
void test2(unsigned int i) {
    aTrace(functArr[gindx++]); // можно без мьютекса, не столь важно какая строка будет
    if (i < deep)
        test2(i+1);
    aeTrace();
}

void test() {
    aTrace("void test()");
    nmTrace("test thread");
    test2(0);
    aeTrace();
}

using namespace std::chrono;
int main()
{
    aTrace("int main()");
    nmTrace("mainThread");
    if (true) {
        functArr = new char*[cnt*deep]; // заполняем псевдостатичный массив для рекурсии
        for (int i = 0; i < cnt*deep; i++) { // чтобы в стек падали разные строки
            char* ch = new char[10];
            itoa(i, ch, 10);
            functArr[i] = ch;
        }
        QFuture<void>* f = new QFuture<void>[cnt];
        
        unsigned __int64 right = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
        for (unsigned int i = 0; i < cnt; i++) // кол-во повторов!
            f[i] = QtConcurrent::run(test);
        for (unsigned int i = 0; i < cnt; i++) // ждём все треды
            f[i].waitForFinished();
        unsigned __int64 left = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
        
        std::cout << std::endl; // сводка
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
