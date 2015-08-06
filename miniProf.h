/*
 * miniProf.h
 *
 *  Created on: 12 juin 2015
 *      Author: duparcj
 */

#ifndef MINIPROF_H_
#define MINIPROF_H_

#include <chrono>
#include <algorithm>
#include <iostream>
#include <stack>
#include <mutex>
#include <map>
#include <thread>
#include <vector>
#include <chrono>

#include <unistd.h> // usleep

namespace miniProf{

#ifdef NO_MINIPROF
#  define STACK
#  define STACK_block(s)
#  define STACK_push(s)
#  define STACK_pop
#  define STACK_print

#  define STACK_record_stop
#  define STACK_record_start
#  define STACK_record_startms(ms)
#else
#  define STACK miniProf::StackFrame sf_##__LINE__(__PRETTY_FUNCTION__);
#  define STACK_block(s) miniProf::StackFrame sf_##__LINE__(s);
#  define STACK_push(s) miniProf::StackFrame::push(">" s);
#  define STACK_pop miniProf::StackFrame::pop();
#  define STACK_print miniProf::StackFrame::print();

#  define STACK_record_stop miniProf::StackFrame::recordStop();
#  define STACK_record_start miniProf::StackFrame::record();
#  define STACK_record_startms(ms) miniProf::StackFrame::record(ms);

struct StackFrame {
	StackFrame(const char* fn);
	~StackFrame();

	static void push(const char* fn);
	static void pop();
	// tools
	static void print();
	static void record(int ms=100);
	static void recordStop();
};


#endif


} //ns miniProf

#endif /* MINIPROF_H_ */
