#include <DVSCapture.h>
#include <D435Capture.h>


#include <thread>
#include <iostream>
#include <experimental/filesystem>
#include <sys/stat.h>

using namespace std;
int main(void)
{

	// folder creation
	time_t now;
	time(&now);
	char folder_c[100];
	sprintf(folder_c, "Capture-%ld", now);
	string folder(folder_c);
    // experimental::filesystem::create_directories(folder);
    mkdir(folder.c_str(), ACCESSPERMS);


    thread t1(DVSMain, folder);

    thread t2(D435Main, folder);

    t1.join();
    t2.join();
    cout << "Over" << endl;
    return 0;
}
