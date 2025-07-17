
#define DEBUG

#ifdef DEBUG
#define Info(msg) std::cout << msg << std::endl;
#else
#define Debug
#endif
