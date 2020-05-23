#include <DVSCapture.h>
#include <D435Capture.h>


#include <thread>
#include <iostream>

using namespace std;
int main(void)
{
    thread t1(DVSMain);

    thread t2(D435Main);

    t1.join();
    cout << "Over" << endl;
    return 0;
}
