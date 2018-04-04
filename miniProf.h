/*
 * miniProf.h
 *
 *  Created on: 12 juin 2015
 *      Author: duparcj
 */

#ifndef MINIPROF_H_
#define MINIPROF_H_

namespace miniProf{

#ifndef NO_MINIPROF

struct StackFrame {
	StackFrame(const char* fn);
	~StackFrame();

	static void push(const char* fn);
	static void pop();
	static void print();

	static void recordStart();
	static void recordStop();

	static void frameStart();
	static void frameStop();
};

#  define MPROF_PASTE_(a,b)         a ## b
#  define MPROF_PASTE(a,b)          MPROF_PASTE_(a,b)
#  define MPROF_func                miniProf::StackFrame MPROF_PASTE(sf_,__LINE__)(__PRETTY_FUNCTION__);
#  define MPROF_block(s)            miniProf::StackFrame MPROF_PASTE(sf_,__LINE__)(s);
#  define MPROF_push(s)             miniProf::StackFrame::push(">" s);
#  define MPROF_pop                 miniProf::StackFrame::pop();
#  define MPROF_print               miniProf::StackFrame::print();
#  define MPROF_record_start        miniProf::StackFrame::recordStart();
#  define MPROF_record_stop         miniProf::StackFrame::recordStop();
#  define MPROF_frame_start         miniProf::StackFrame::frameStart();
#  define MPROF_frame_end           miniProf::StackFrame::frameStop();

#ifndef MPROF_out
#  define MPROF_out(x)              std::cerr<<xstd::endl;
#endif
#ifndef MPROF_barsize               // nb of char shown in the graphic bar
#  define MPROF_barsize             20
#endif
#ifndef MPROF_ignore_root           // don't show the root element (uncatched samples)
#  define MPROF_ignore_root         0
#endif
#ifndef MPROF_sampling_us			// sample period
#  define MPROF_sampling_us         10000
#endif
#ifndef MPROF_show_sample_count
#  define MPROF_show_sample_count   1
#endif
#ifndef MPROF_show_percent
#  define MPROF_show_percent        1
#endif
#ifndef MPROF_show_perframe_us
#  define MPROF_show_perframe_us     1
#endif


#else
#  define MPROF_func
#  define MPROF_block(s)
#  define MPROF_push(s)
#  define MPROF_pop
#  define MPROF_print
#  define MPROF_record_start
#  define MPROF_record_stop
#  define MPROF_frame_start
#  define MPROF_frame_end

#endif /* NO_MINIPROF */

} //ns miniProf

#ifndef NO_MINIPROF
#  ifdef MPROF_implement
#    include "miniProf.hpp"
#  endif
#endif

#endif /* MINIPROF_H_ */

