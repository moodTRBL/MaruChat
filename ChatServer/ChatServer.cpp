#include "pch.h"
#include "ThreadManager.h"
#include "Allocator.h"
#include "Memory.h"

class Test {
public:
	Test() {
		cout << "init" << endl;
	}
	~Test() {
		cout << "delete" << endl;
	}
	int32 value = 0;
};

int main() {
	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([]() {
			while (true) {
					Test* test = xnew<Test>();
					this_thread::sleep_for(10ms);
					xdelete(test);
				}});
	}
	GThreadManager->Join();

	return 0;
}