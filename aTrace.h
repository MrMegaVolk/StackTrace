#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <QDebug>

#include <iostream>
#include <thread>
#include <mutex>

#define nmTrace(str) stackTrace::nameTrace(std::this_thread::get_id(), str)

#define aTraceTry(str) \
    stackTrace::pushTrace(std::this_thread::get_id(), __FILE__, str, __LINE__); \
    try {
#define aeTraceCatch() } catch(...) { \
    throw; \
    } stackTrace::popTrace(std::this_thread::get_id())

#define aTrace(str) stackTrace::pushTrace(std::this_thread::get_id(), __FILE__, str, __LINE__)
#define aeTrace() stackTrace::popTrace(std::this_thread::get_id())

#define pTrace() stackTrace::printTrace()

#define MAX_TRACE_COUNT 1000 // threadCount for indx
#define MAX_TRACE_DEPTH 1000 // stackDepp -> __FILE__, function, __LINE__

namespace stackTrace
{
    static char* undef = "undefined";
    typedef std::thread::id tid;
    struct stackTraceStruct {
        stackTraceStruct() : name(undef), last(-1) {
            files = new char*[MAX_TRACE_DEPTH];
            funct = new char*[MAX_TRACE_DEPTH];
            lines = new int  [MAX_TRACE_DEPTH];
        }

        tid   id;
        char* name;
        int   last;
        char** files; // __FILE__
        char** funct; // function
        int *  lines; // __LINE__
        //std::mutex insertMutex;
    };
    struct stackTraceStruct2 {
        stackTraceStruct2() : name(undef), last(-1) {}                    
                             
        tid   id;            
        char* name;          
        int   last;          
        char* files[MAX_TRACE_DEPTH]; // __FILE__
        char* funct[MAX_TRACE_DEPTH]; // function
        int   lines[MAX_TRACE_DEPTH]; // __LINE__
        //std::mutex insertMutex;
    };
    //static stackTraceStruct* threadmap = new stackTraceStruct[MAX_TRACE_COUNT];
    static stackTraceStruct threadmap[MAX_TRACE_COUNT];
    static std::mutex insertMutex;

    static void nameTrace(tid id, char* name) {
        int indx = 0;
        bool found = false;
        for (; indx < MAX_TRACE_COUNT; indx++) // поиск, можно без мьютекса
            if (threadmap[indx].id == id) { // нашли открытый тред
                threadmap[indx].name = name; // можно не лочить, шанс асинхронного доступа почти нулевой
                found = true;
                break;
            }
        if (!found and false) { // если не нашли, то и смысла именовать нету
            int indx = 0;
            insertMutex.lock(); // потому что тут важно убрать коллизию
            for (; indx <= MAX_TRACE_COUNT; indx++)
                if (threadmap[indx].last == -1) { // если дошли до конца заполненого списка
                    threadmap[indx].id = id;
                    threadmap[indx].name = name;
                    found = true;
                    break;
                }
            insertMutex.unlock();
        }
        if (!found)
            qDebug() << "WARNING! Filed set name:" << name;
    };

    static void pushTrace(tid id, char* file, char* str, int line) {
        int indx = 0;
        bool found = false;
        for (; indx < MAX_TRACE_COUNT; indx++) // поиск, можно без мьютекса
            if (threadmap[indx].id == id) { // нашли открытый тред
                //mutex.lock(); // каждый тред будет писать в свою структуру, мьютекс лишний
                found = true;
                if (threadmap[indx].last + 1 >= MAX_TRACE_DEPTH) { // если стек полон
                    qDebug() << "WARNING! Reached maximum of stack!" << threadmap[indx].name;
                    break;
                }

                threadmap[indx].last++;
                threadmap[indx].files[threadmap[indx].last] = file;
                threadmap[indx].funct[threadmap[indx].last] = str;
                threadmap[indx].lines[threadmap[indx].last] = line;
                //mutex.unlock();
                break;
            }
        if (!found) {
            int indx = 0;
            insertMutex.lock(); // потому что тут важно убрать коллизию
            for (; indx < MAX_TRACE_COUNT; indx++)
                if (threadmap[indx].last == -1) { // если дошли до конца заполненого списка
                    threadmap[indx].id = id;
                    threadmap[indx].name = undef;
                    threadmap[indx].last++;
                    threadmap[indx].files[threadmap[indx].last] = file;
                    threadmap[indx].funct[threadmap[indx].last] = str;
                    threadmap[indx].lines[threadmap[indx].last] = line;
                    found = true;
                    break;
                }
            insertMutex.unlock();
        }
        if (!found)
            qDebug() << "WARNING! Filed pushTrace on new thread";
    };
    static void popTrace(tid id) {
        int indx = 0;
        bool found = false;
        for (; indx < MAX_TRACE_COUNT; indx++) {
            if (threadmap[indx].id == id) { // нашли открытый тред
                //mutex.lock(); каждый тред в свою структуру
                if (threadmap[indx].last >= 0) {
                    found = true;
                    threadmap[indx].files[threadmap[indx].last] = NULL;
                    threadmap[indx].funct[threadmap[indx].last] = NULL;
                    threadmap[indx].lines[threadmap[indx].last] = NULL;
                    threadmap[indx].last--;
                }
                //mutex.unlock();
                break;
            }
        }
        if (!found)
            qDebug() << "WARNING! Filed popTrace on new thread";
    };

    static void printTrace() {
        insertMutex.lock();
        std::cout << "Stack trace with all threads:" << std::endl;
        for (int j=0; j<MAX_TRACE_COUNT; j++) {
            char* prev = nullptr;
            if (threadmap[j].last == -1) {
                //std::cout << "\t\tIs Empty!" << std::endl;
                continue;
            }
            std::cout << "\t" << threadmap[j].name << std::endl;
            for (int i = 0; i < MAX_TRACE_DEPTH; i++) {
                if (prev == nullptr || strcmp(prev, threadmap[j].files[i]) != 0) {
                    std::cout << "\t\t" << threadmap[j].files[i] << std::endl;
                    prev = threadmap[j].files[i];
                }
                std::cout << "\t\t\t" << threadmap[j].funct[i]
                          << " on line " << threadmap[j].lines[i] << std::endl;
                if (i+1 > threadmap[j].last)
                    break;
            }
        }
        insertMutex.unlock();
    };
};

#endif // STACKTRACE_H
