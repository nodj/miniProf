#include "miniProf/miniProf.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <ostream>
#include <stack>
#include <thread>
#include <map>
#include <vector>
#include <algorithm>
#include <ostream>
#include <iomanip>


namespace miniProf{


struct Context{
	static std::stack<const char*> callStack;
	static std::map<const char*, int> topMap;
	static std::thread stackRecorder;
	static bool stopRecord = false;
	static std::mutex m;
	static uint64_t recordTimeStart;
	static uint32_t frameCount;
	static uint32_t frameTrack[2];
	static uint8_t  inFrameState;
};

void recordJob();

typedef std::pair<const char*, int> countedSF;


static inline uint64_t now(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
}

StackFrame::StackFrame(const char* fn) { push(fn); }
StackFrame::~StackFrame() { pop(); }

void StackFrame::push(const char* fn){
	std::lock_guard<std::mutex> l(m);
	callStack.push(fn);
}

void StackFrame::pop() {
	std::lock_guard<std::mutex> l(m);
	callStack.pop();
}

// tools
void StackFrame::print(){
	MPROF_out(callStack.top());
}

void StackFrame::recordStart(){
	push("root");
	miniProf::stopRecord = false;
	miniProf::frameCount = 0;
	miniProf::stackRecorder = std::thread(recordJob);
	miniProf::recordTimeStart = now();
}

void StackFrame::recordStop() {
	pop(); // root
	miniProf::stopRecord = true;
	miniProf::stackRecorder.join();
	uint64_t recordTimeStop = now();
	uint64_t recordTime_ms = recordTimeStop-recordTimeStart;

	auto rootSamples = topMap["root"];
	if(MPROF_ignore_root){topMap.erase("root");}
	std::vector<countedSF> listing(topMap.begin(), topMap.end());

	auto inv_sort_pair = [](const countedSF& l, const countedSF& r){return l.second > r.second;};
	std::sort(listing.begin(), listing.end(), inv_sort_pair);

	std::vector<float> percents;
	percents.reserve(listing.size());

	int sampleCount = 0;
	for (auto i = listing.cbegin(); i < listing.cend(); i++)
	{ sampleCount += i->second; }

	for (auto i = listing.cbegin(); i < listing.cend(); i++)
	{ percents.push_back(float(i->second) / sampleCount); }

	float maxPercent = *std::max_element(percents.begin(), percents.end());

	std::ostringstream ss;
	ss << "----- miniProf::SAMPLER -----" << std::endl;

	// sample line
	ss << "SAMPLES #" << sampleCount;
	if(MPROF_ignore_root) ss << " (+" << rootSamples<<")";
	ss << ", stepping(us): " << MPROF_sampling_us;
	ss << ", total(s): " << recordTime_ms*1e-3;
	ss << std::endl;

	//frame line
	uint32_t samplesCount = miniProf::frameTrack[0]+miniProf::frameTrack[1];
	float frameStride_us = 1e3*float(recordTime_ms)/miniProf::frameCount;
	float inFrameRatio = float(miniProf::frameTrack[1])/samplesCount;
	float inFrameTime_us = inFrameRatio*frameStride_us;
	ss << "FRAMES #"       << miniProf::frameCount;
	ss << ", stride(us): " << frameStride_us;
	ss << ", fps: "        << 1e6/frameStride_us;
	ss << ", used(us): "   << inFrameTime_us;
	ss << ", used(pct): "  << 100.*inFrameRatio;
	ss << std::endl;
	int i = 0;

	// detail
	int sample_count_width = 0;
	int perFrame_us_width = 0;
	for(auto csf = listing.cbegin(); csf<listing.cend(); csf++) {
		int pct_i = int(0.5+100.*percents[i]);
		if(pct_i<1) break; // filter low impact fcts
		int sampleCount = csf->second;
		int perFrame_us = percents[i]*inFrameTime_us;
		if(sample_count_width==0){
			sample_count_width = 1+std::log10(sampleCount);
			perFrame_us_width  = 1+std::log10(perFrame_us);
		}

		// draw bar
		static int N = MPROF_barsize;
		int nbChar = int(0.5+N*percents[i]/maxPercent);
		auto f = ss.fill();
		ss << '[';
		ss << std::setfill(' ') << std::setw(N-nbChar) << ""; // "" is normal!
		ss << std::setfill('#') << std::setw(nbChar)   << ""; // "" is normal!
		ss << "] ";
		ss.fill(f);

		// numerical stats
		if(MPROF_show_sample_count) ss << std::setw(sample_count_width) << sampleCount << "smp ";
		if(MPROF_show_percent)      ss << std::setw(2)                  << pct_i << "% ";
		if(MPROF_show_perframe_us)  ss << std::setw(perFrame_us_width)  << perFrame_us << "us ";
		ss << "@ ";
		ss << csf->first << std::endl; // fct name
		i++;
	}
	MPROF_out(ss.str())
}

void StackFrame::frameStart() {
	miniProf::inFrameState = true;
}
void StackFrame::frameStop() {
	miniProf::frameCount++;
	miniProf::inFrameState = false;
}

void recordJob(){
	while(!stopRecord){
		miniProf::m.lock();
		auto top = callStack.top();
		miniProf::m.unlock();
		miniProf::topMap[top]++;
		miniProf::frameTrack[miniProf::inFrameState]++;
	    std::this_thread::sleep_for(std::chrono::microseconds(MPROF_sampling_us));
	}
}


} //ns miniProf

