#ifndef OUTPUT_H
#define OUTPUT_H
#include <fstream>

std::ofstream MonitorOut("Monitor.txt", std::ofstream::app);

std::ofstream OverOut("Over.txt", std::ofstream::app);

#endif