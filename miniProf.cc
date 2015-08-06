#include "miniProf.h"
#include <cmath>

namespace miniProf{

static std::stack<const char*> callStack;
static std::map<const char*, int> topMap;
static std::thread stackRecorder;
static bool stopRecord = false;
static int steppingms = -1;
static std::mutex m;
static uint64_t recordStart;

void recordJob(int ms);

typedef std::pair<const char*, int> countedSF;

struct inv_sort_pair_t {
	bool operator()(const countedSF& l, const countedSF& r)
	{ return l.second > r.second; }
} inv_sort_pair;

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
	std::cout << callStack.top() << std::endl;
}

void StackFrame::record(int ms){
	steppingms=ms;
	stackRecorder = std::thread(recordJob, ms);
	recordStart = now();
}

void StackFrame::recordStop() {
	miniProf::stopRecord = true;
	miniProf::stackRecorder.join();

	uint64_t stop = now();

	std::vector<countedSF> listing(topMap.begin(), topMap.end());
	std::sort(listing.begin(), listing.end(), inv_sort_pair);

	int sampleCount=0;

	for(auto i = listing.cbegin(); i<listing.cend(); i++)
	{ sampleCount += i->second; }

	std::vector<float> percents;
	percents.reserve(listing.size());

	for(auto i = listing.cbegin(); i<listing.cend(); i++)
	{ percents.push_back(float(i->second)/sampleCount); }

	float maxPercent = *std::max_element(percents.begin(), percents.end());

	std::cerr << "----- miniProf::SAMPLER -----" << std::endl;
	std::cerr << "samples: "        << sampleCount;
	std::cerr << ", stepping(ms): " << steppingms;
	std::cerr << ", total(s): "    << float(stop-recordStart)*1e-3 << std::endl;
	int i = 0;

	for(auto csf = listing.cbegin(); csf<listing.cend(); csf++)
	{
		static int N = 15;
		int nbChar = std::round(N*percents[i]/maxPercent);
		std::cerr << '[';
		for(int j=0; j<nbChar; ++j){ std::cerr << '>';}
		for(int j=nbChar; j<N; ++j){ std::cerr << ' ';}
		std::cerr << "] ";
		std::cerr << csf->second << '\t' << csf->first<< std::endl;
		i++;
	}
}

void recordJob(int ms){
	int us = 1000*ms;
	while(!stopRecord){
		m.lock();
		auto top = callStack.top();
		m.unlock();

		topMap[top]++;
		usleep(us);
		//		std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // too much c++11...
	}
}


} //ns miniProf

