#ifndef MACROUTIL_H_
#define MACROUTIL_H_

#ifndef MAX
#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))
#endif

#ifndef ERREXIT
#define ERREXIT(format, ...) do{fprintf(stderr, format "\n", ##__VA_ARGS__);exit(EXIT_FAILURE);}while(0)
#endif

#ifndef DBGPRINT
#define DBGPRINT(debug, format, ...) if ((debug)) fprintf(stdout, format "\n", ##__VA_ARGS__)
#endif

#ifndef DBGFUNC
#define DBGFUNC(debug, function) if ((debug)) (function)
#endif

#endif /* MACROUTIL_H_ */
