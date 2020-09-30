#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <experimental\filesystem>

#ifdef _DEBUG
#pragma comment(lib, "opencv_highgui249d.lib")
#pragma comment(lib, "opencv_core249d.lib")
#else
#pragma comment(lib, "opencv_highgui249.lib")
#pragma comment(lib, "opencv_core249.lib")
#endif

using namespace cv;
using namespace std;

struct Event {
	uint16_t x;
	uint16_t y;
	unsigned char polar;
};

bool write_buf(ofstream& of, const vector<Event>& buf) {
	if (buf.empty()) return false;

	unsigned long n_event = (unsigned long)buf.size();
	of.write((char*)(&n_event), sizeof(n_event));
	of.write((char*)(buf.data()), buf.size() * sizeof(Event));
	return true;
}

std::size_t number_of_files_in_directory(std::experimental::filesystem::path path) {
	using namespace std::experimental;
	using filesystem::directory_iterator;
	using fp = bool(*)(const filesystem::path&);
	return std::count_if(directory_iterator(path), directory_iterator{}, (fp)filesystem::is_regular_file);
}

int main(int argc, char**argv) {
	if (argc == 1) {
		printf("Usage: dvs_formater dir\n");
		return 0;
	}

	string raw_folder = argv[1];
	int n_files = (int)number_of_files_in_directory(raw_folder);
	time_t now;	time(&now);
	char file_name[100];
	sprintf(file_name, "dvs-%llu.bin", now);
	//	string folder(file_name);
	//#ifdef __linux
	//	mkdir(folder.c_str(), ACCESSPERMS);
	//#else
	//	experimental::filesystem::create_directories(folder); // for vs2017
	//#endif 

	const int FRAME_DT = 20000;	// 20msec
	ofstream event_writer(file_name, std::ios::binary);
	vector<Event> event_buf;
	for (int k = 0; k < n_files; k++) {
		string file_name = raw_folder + "DVS_raw_frame" + to_string(k) + ".raw";
		ifstream file(file_name, ios::binary | ios::ate);
		int length = (int)file.tellg();
		file.seekg(0);
		unsigned char raw_buf[200000] = { 0 };
		file.read((char*)raw_buf, length);

		if (length < 4) continue;
		if (length % 4 != 0) length -= length % 4;

		printf(" - Parse packet %d, len=%d\n", k, length);
		unsigned int longTs = 0, shortTs = 0;
		unsigned long long 	timeStamp = 0, T0 = 0;
		Mat image(480, 640, CV_8UC1, Scalar(128));
		unsigned char *ptr = raw_buf;

		for (int i = 0; i < length; i += 4) {
			unsigned short posX, posY, posY0, grpAddr, header;
			header = ptr[i] & 0x7C;	// 0111 1100
			if (ptr[i] & 0x80) {	// Group Events Packet
				grpAddr = (ptr[i + 1] & 0xFC) >> 2;
				unsigned char polarity;
				if (ptr[i + 3]) {
					posY0 = grpAddr << 3;
					polarity = (ptr[i + 1] & 0x01) ? 1 : 0;
					for (int n = 0; n < 8; n++) {
						if ((ptr[i + 3] >> n) & 0x01) {
							posY = posY0 + n;
							image.at<char>(posY, posX) = polarity * 255;
							Event evt{ posX, posY, polarity };
							event_buf.push_back(evt);
							//printf("(%d, %d): %d\n", posX, posY, polarity);
						}
					}
				}
				if (ptr[i + 2]) {
					grpAddr += (header >> 2);	// Offset
					posY0 = grpAddr << 3;
					polarity = (ptr[i + 1] & 0x02) ? 1 : 0;
					for (int n = 0; n < 8; n++) {
						if ((ptr[i + 2] >> n) & 0x01) {
							posY = posY0 + n;
							image.at<char>(posY, posX) = polarity * 255;
							Event evt{ posX, posY, polarity };
							event_buf.push_back(evt);
							//printf("(%d, %d): %d\n", posX, posY, polarity);
						}
					}
				}
			}
			else {
				switch (header) {
				case (0x04):	// 0000 01** | --ST TTTT | TTTT T-CC | CCCC CCCC	Column Address (10) + SubTimestamp (10)
					if (ptr[i + 1] & 0x20) {	// Update timestamp only once when new frame starts
						shortTs = ((ptr[i + 1] & 0x1F) << 5) | ((ptr[i + 2] & 0xF8) >> 3);
						timeStamp = longTs + shortTs;
						printf("ts = %d + %d\n", longTs, shortTs);

						write_buf(event_writer, event_buf);
						event_buf.clear();
						event_writer.write((char*)(&timeStamp), sizeof(timeStamp));

						// update event image
						if (((timeStamp - T0) > FRAME_DT) || (timeStamp < T0)) {
							imshow("DVS", image);
							cv::waitKey(1);
							image.setTo(cv::Scalar(128));
							T0 = timeStamp;
						}
					}
					posX = (((ptr[i + 2] & 0x03) << 8) | (ptr[i + 3] & 0xFF));		// Original
					break;

				case (0x08):	// 0000 10** | --TT TTTT | TTTT TTTT | TTTT TTTT	Reference Timestamp (22)
					longTs = (((ptr[i + 1] & 0x3F) << 16) | ((ptr[i + 2] & 0xFF) << 8) | (ptr[i + 3] & 0xFF)) * 1000;
					break;

				case (0x40):	// 0100 00** | --II IIII | IIII IIII | IIII IIII	Packet ID (22)
								// Packet ID is used to check packet loss (MIPI)
								//packetID = ((buf[i + 1] & 0x3F) << 26) | ((buf[i + 2] & 0xFF) << 18) | ((buf[i + 3] & 0xFF) << 10);
					break;
				case (0x00):	// 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000	Padding (MIPI)
								//i = pktlen;	// ignore all the remaining packet data
					break;
				default:
					break;
				}
			}
		}
	}
	write_buf(event_writer, event_buf);

	cout << "Over" << endl;
}
