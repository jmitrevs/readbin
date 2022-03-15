// The general header file for the readbin program


//const int READ_SIZE = 0x8000000;
const long READ_SIZE =  0x1d00100;
const long WRITE_SIZE = 0x100000;

// forward definition to process at most numToRead events from readbuf;
// actual number read and written is output in the two passed in variables.
// (Those are overwritten, so the input value is not used)
void process(uint8_t readbuf[READ_SIZE], uint8_t writebuf[WRITE_SIZE], int numToRead, off_t& numRead, off_t& numWritten);

